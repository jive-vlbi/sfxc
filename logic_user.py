import re
import operator
import logic
from logic import *

def chain(*iterables):
    for iterable in iterables:
        for item in iterable:
            yield item


def iand(c1, c2):
  for i in c1:
    for j in c2:
      yield [i, j]
 
  
def matching(iterator, pattern=None):
  if pattern == None:
    for i in iterator:
      yield i
  else:
    for i in iterator:
      if pattern.match(i):
        yield i

#for i in matching(inputnode(), Like("_state")):
#  print "print testing matching: " + `i`
  
class ByOrder():
  def __init__(self, a):
      self.a = a
      self.data = []
      
  def add(self):
    if isinstance(self.a, UnifyingVariable):
      data.append(a._value)  

def sort2(iterator, keys, oper):
  for i in iterator:
    k.add()
    
  k.data.sort(key=oper)     
  for i in k.data:
    yield i


def sort(iterator, keyd=operator.itemgetter(0)):
  table = []
  for i in iterator:
    table.append(i)
  
  table.sort()     
  for i in table:
    yield i



def query(Subject=UnifyingVariable(), Predicate=UnifyingVariable(), Object=UnifyingVariable()):
  global database
  for i in database:
    for l1 in generalUnify(Subject, i[0]):
      for l2 in generalUnify(Predicate, i[1]):
        for l3 in generalUnify(Object, i[2]):
          yield [l1, l2, l3]

def inputnode():
  for i in query(Predicate="is_a", Object="inputnode"):
    yield i

def correlationnode():
  for i in query(Predicate="is_a", Object="correlationnode"):
    yield i
    

def inputdir():
  for i in query(Subject="gen-runtime", Predicate="inputdir", Object=UnifyingVariable()):
    yield i[2]
    break
    
def cacheddir():
  for i in query(Subject="gen-runtime", Predicate="inputdir", Object=UnifyingVariable()):
    yield i[2]
    break
    

database = []
def add_to_database(database, string):
  if re.search(".meta", string):
    #print "The file to parse is => "+string
    fp = open(string, "r")
    
    string = re.compile("#").sub("_", fp.read())
    db = eval( string )
    [database.append(i) for i in db if not database.count( i )]  
    #database = db2 | database
  else:
    None
    #print "Skipping => "+string
   
   

