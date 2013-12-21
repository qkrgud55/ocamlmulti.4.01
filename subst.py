
import os, re
import sys
import termios
import fcntl

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

keyset = set()
typeset = set()
funset = set()
varset = set()

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
  f = open("/home/phc/ocaml_internal/ocamlmulti/reentrant_def", "r")
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
  f = open("/home/phc/ocaml_internal/ocamlmulti/reentrant_var", "r")
  count = 0
  while True:
    line = f.readline()
    if not line: break
    m = re.match(r'^([^:]*):.*$', line)
    if not m: print line
    filepath = m.groups()[0]
    m = re.search(r'ctx->(([a-zA-Z0-9_])+)[^a-zA-Z0-9_]', line)
    if not m: print line
    var = m.groups()[0]
#    print filepath, 'ctx->', var
    varset.add(var)
    
    count += 1
  print "# of member vars of context :", count
  print varset

parse_reentrant_def1()
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
  pass
  
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
      handle_line_fun(filepath,lines,count,lines[count])
      count += 1

def traverse():
  basedir = '/home/phc/ocaml_internal/ocamlmulti401'
  for root, dirs, files in os.walk(basedir + "/byterun"):
    traverse_fun(root,dirs,files)
  for root, dirs, files in os.walk(basedir + "/asmrun"):
    traverse_fun(root,dirs,files)

traverse()
