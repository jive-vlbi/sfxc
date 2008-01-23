# 2007, Written by Nico Kruithof <Kruithof@jive.nl>

import sys
import vex_parser
import MultiDict

def Vex(filename):
  root = vex_parser.parse_vex(str(filename))
  return __convert(root)


def __convert(node):
  if node.type() == "STRING":
    return node.to_string()
  if node.type() == "ARRAY":
    return [__convert(elem) for elem in node]
  if node.type() == "DICT":
    result = MultiDict.MultiDict()
    for key in node.keys():
      for value in node.values(key):
        result.append(key, __convert(value))
    return result
  print "oops not here I hope"
  sys.exit(1);
