import sys

def info(object, spacing=10, collapse=1):
	"""Print methods and doc strings.

	Takes module, class, list, dictionary, or string."""
	methodList = [e for e in dir(object) if callable(getattr(object, e))]
	processFunc = collapse and (lambda s: " ".join(s.split())) or (lambda s: s)
	print "\n".join(["%s %s" %
					 (method.ljust(spacing),
					  processFunc(str(getattr(object, method).__doc__)))
					 for method in methodList])

def generalGetValue(value):
    if isinstance(value, UnifyingVariable):
        if not value._isBound:
            return value
        else:
            return value._value
    else:
        return value

def generalUnify(arg1, arg2):
    arg1Value = generalGetValue(arg1)
    arg2Value = generalGetValue(arg2)
    if isinstance(arg1Value, UnifyingVariable):
        for l1 in arg1Value.unify(arg2Value):
            yield l1
    elif isinstance(arg2Value, UnifyingVariable):
        for l1 in arg2Value.unify(arg1Value):
            yield l1
    else:
        # Arguments are "normal" types.
        if arg1Value == arg2Value:
            yield arg1Value
            
class UnifyingVariable:
    def __init__(self):
        self._isBound = False
        self._value = None

    def unify(self, arg):
        if not self._isBound:
            self._value = arg
            self._isBound = True
            yield arg
            # Remove the binding.
            self._isBound = False
        elif self._value == arg:
            yield arg
            
            

  
def triple(Subject=UnifyingVariable(), Predicate=UnifyingVariable(), Object=UnifyingVariable()):
  global db
  for i in db:
    for l1 in generalUnify(Subject, i[0]):
      for l2 in generalUnify(Predicate, i[1]):
        for l3 in generalUnify(Object, i[2]):
          yield [l1, l2, l3]

def is_a(Subject=UnifyingVariable(), Object=UnifyingVariable()):
  global db
  functioname = sys._getframe().f_code.co_name
  for i in db:
    if i[1] == functioname:
      for l1 in generalUnify(Subject, i[0]):
        for l2 in generalUnify(Object, i[2]):
          yield [l1, l2]
                  
                  
def human(Person=UnifyingVariable()):
  functioname = sys._getframe().f_code.co_name
  global database
  for i in database[functioname]:
    for ll in generalUnify(Person, i):
      yield ll

def brother(PersonA, PersonB):
  functioname = sys._getframe().f_code.co_name
  global database
  for i in database[functioname]:
    for l1 in generalUnify(PersonA, i[0]):
      for l2 in generalUnify(PersonB, i[1]):
        yield [l1, l2]
  
def brother(PersonA, PersonB):
  functioname = sys._getframe().f_code.co_name
  global database
  for i in database[functioname]:
    for l1 in generalUnify(PersonA, i[0]):
      for l2 in generalUnify(PersonB, i[1]):
        yield [l1, l2]
  
  
#    
#for i in human():
#  print "Method2: " + `i`
  
  
  
#for l2 in brother("Damien", UnifyingVariable()):
#  print "They are brother: "+`l2`

#var = UnifyingVariable()  
#for i in human(var):
#  print "Var:"+ `var._value`
  
#for i in is_a(Object="channel_extractor"):
#  print "Type:"+`i`  
 
#for i in is_a(Object="datafile"):  
#  print "Type:"+`i`  
  
#for i in triple(Predicate="is_a"):
#  print "Result:" + `i`
