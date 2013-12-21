
import os, re



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

def parse_reentrant_def1():
  f = open("/home/phc/ocaml_internal/ocamlmulti/reentrant_def", "r")
  count = 0
  while True:
    line = f.readline()
    if not line: break
    m = re.match(r'^([^:]*):((\w+\s)?)\s*(\w+|\w+ \*|struct \w+)\s*(\w+)\s*\(.*$', line)
    if not m: print line
    count += 1
  print count
  

def traverse():
  for root, dirs, files in os.walk("/home/phc/ocaml_internal/ocamlmulti401"):
    print root
    for file_name in files:
      print os.path.join(root, file_name)

parse_reentrant_def1()

