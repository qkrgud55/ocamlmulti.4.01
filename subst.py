
import os, re
import sys
import termios
import fcntl
from termcolor import colored

def getch():
    fd = sys.stdin.fileno()

    oldterm = termios.tcgetattr(fd)
    newattr = termios.tcgetattr(fd)
    newattr[3] = newattr[3] & ~termios.ICANON & ~termios.ECHO
    termios.tcsetattr(fd, termios.TCSANOW, newattr)

    oldflags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, oldflags | os.O_NONBLOCK)

    try:        
        while 1:            
            try:
                c = sys.stdin.read(1)
                break
            except IOError: pass
    finally:
        termios.tcsetattr(fd, termios.TCSAFLUSH, oldterm)
        fcntl.fcntl(fd, fcntl.F_SETFL, oldflags)
    return c

def printn(s):
  print s,
  sys.stdout.write('')

keyset = set()
typeset = set()
funset = set()
varset = set()

skip_num = 0
no_count = 0

def parse_reentrant_def():
  f = open("/home/phc/ocaml_internal/ocamlmulti/reentrant_def", "r")
  data = f.read()
  
  it = 0
  count = 0
  prefix = set()
  while it < len(data):
    # m = re.search(r'^([^:]*):((\w+)?)\s+.(\w+)*\s+(\w+)\s*\(.*$',data[it:])
    m = re.search(r'^([^:]*):((\w+)?)\s+(\w+).*$',data[it:],re.MULTILINE)
    if not m: break
#    print data[it+m.span()[0]:it+m.span()[1]]
#    prefix.add(m.groups()[3])
    it += m.span()[1]
    count += 1
  print count
  print prefix

def none_str(s):
  if not s: return ""
  return s

def parse_reentrant_def1():
  reentrant_funs = set()
  f = open("reentrant_def", "r")
  count = 0
  while True:
    line = f.readline()
    if not line: break
    m = re.match(r'^([^:]*):(((\w+)\s)?)\s*((\w+|struct \w+|unsigned \w+)\s|(\w+|struct \w+|unusigned \w+)\s*\*)\s*([^\s*]+)\s*(\(.*)$', line)
    if not m: print line
    mg = map(str.strip, map(none_str, m.groups()))
#    print "%30s: %15s %30s %40s  %s" % (mg[0], mg[1], mg[4], mg[7], mg[8])
    if re.match(r'.*_r', mg[7]):
      funset.add(mg[7][:-2])
    keyset.add(mg[1])
    typeset.add(mg[4])
    count += 1
  print "# of reentrant fun def :", count
  print funset 
  print '='*50
  print keyset
  print '='*50
  print typeset
  print '='*50
  
def parse_reentrant_var():
  reentrant_vars = set()
  f = open("reentrant_var", "r")
  count = 0
  while True:
    line = f.readline()
    if not line: break

    m = re.match(r'^([^:]*):.*$', line)
    if not m: print line
    filepath = m.groups()[0]
    for m in re.findall(r'ctx->(([a-zA-Z0-9_])+)[^a-zA-Z0-9_]', line):
      var = m[0]
      print filepath, 'ctx->', var
      varset.add((filepath.split('/')[-1],var))
      count += 1
      
  print "# of member vars of context :", count

  # manual
  varset.difference_update(set([('md5.c','buf'), ('md5.c','in')]))
  print varset

#parse_reentrant_def1()
parse_reentrant_var()

def handle_line_fun(filepath, lines, idx, line):
  m = re.search(r'(((\w+)\s)?)\s*((\w+|struct \w+|unsigned \w+)\s|(\w+|struct \w+|unusigned \w+)\s*\*)\s*([^\s*]+)\s*\(([^)]*)', line)
  if not m: 
#    print 'failed :', line
    return
  mg = map(str.strip, map(none_str, m.groups()))
  if mg[6] not in funset: return
  if mg[0] not in keyset: return
  if mg[3] in ['return']: return
  print '\n'*3
  print 'found  :', line
  print "%60s: %15s %30s %40s ( %s" % (filepath, mg[0], mg[3], mg[6], mg[7])
  print '\n'*3

  start = idx - 10
  if start < 0: start = 0
  end = idx + 10
  if end > len(lines):
    end = len(lines)
  for it in range(start,end):
    print '%4d :' % (it), lines[it]
    if it==idx:
      print 'found:', ' '*m.regs[7][0] + '='*len(mg[6])

  parts = mg[7].split()
  if len(parts)==0:
    no_param = True
  elif len(parts)==2 and parts[0]=='void':
    no_param = True
  elif parts==['void']:
    no_param = True
  elif parts[0]=='pctx':
    return
  else:
    no_param = False

  offset_insert = m.regs[8][0]
  offset_cont = m.regs[8][1]
  if no_param:
    result_line = line[:offset_insert] + 'pctx ctx' + line[offset_cont:]
  else:
    result_line = line[:offset_insert] + 'pctx ctx, ' + line[offset_insert:]

  print 'result','-'*30
  print result_line

  print '(yes/no/quit)? :',
  key = getch()
  if key=='q':
    return exit()
  elif key=='y':
    lines[idx] = result_line
    fp = file(filepath, 'w')
    fp.write('\n'.join(lines))
    fp.close()

def handle_line_var(filepath, lines, idx, line):
  global no_count, skip_num

  it = 0
  while True:
    m = re.search(r'[a-zA-Z0-9_]+', line[it:])
    if not m: break
    symbol = m.group()
    if (filepath.split('/')[-1],symbol) not in varset:
      it += m.regs[0][1]
      continue
    offset = it+m.regs[0][0]
    if offset >= len('ctx->') and re.match(r'.*ctx->$', line[:offset]):
      it += m.regs[0][1]
      continue

    print '\n'*1
    print filepath
    print '-'*50
    
    start = idx - 15
    if start < 0: start = 0
    end = idx + 15
    if end > len(lines):
      end = len(lines)
    for it1 in range(start,end):
      if it1==idx:
        printn(colored('%4d : ' % it1, 'red'))
        printn(line[:offset]) 
        printn(colored(line[offset:offset+len(symbol)],'green')) 
        printn(line[offset+len(symbol):])
        print ''
        printn('found: '+' '*(offset))
        print colored('='*len(symbol), 'yellow')
      else:
        print '%4d :' % (it1), lines[it1]

    result_line = line[:offset] + "ctx->" + line[offset:]
    print '\n'*1
    print 'result','-'*30
    printn(line[:offset])
    printn(colored("ctx->"+symbol,'green'))
    print line[offset+len(symbol):]

    if no_count < skip_num:
      no_count += 1
      it += m.regs[0][1]
    else:
      print '(yes/no/quit)? :',
      key = getch()
      if key=='q':
        fp = file('no_count','w')
        fp.write(str(no_count))
        fp.close()
        return exit()
      elif key=='y':
        lines[idx] = result_line
        fp = file(filepath, 'w')
        fp.write('\n'.join(lines))
        fp.close()
        it += m.regs[0][1]+len('ctx->')
        line = result_line
      else:
        no_count += 1
        it += m.regs[0][1]

def traverse_fun(root, dirs, files):
  print root
  for file_name in files:
    if not re.match(r'.*\.[ch]', file_name):
      continue
    filepath =  os.path.join(root, file_name)
    print filepath
    lines = file(filepath, 'r').read().split('\n')
#      print '\n'.join(lines[:20])
#      tmp = raw_input('press enter to continue')
    count = 0
    offset = 0
    while count < len(lines):
      handle_line_var(filepath,lines,count,lines[count])
      count += 1

def traverse():
  basedir = './'
  for root, dirs, files in os.walk(basedir + "/byterun"):
    traverse_fun(root,dirs,files)
  for root, dirs, files in os.walk(basedir + "/asmrun"):
    traverse_fun(root,dirs,files)

fp = file('no_count', 'r')
skip_num = int(fp.read())

traverse()
