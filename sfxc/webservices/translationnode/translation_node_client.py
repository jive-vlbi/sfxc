import sys
import simplejson as json
from NewTranslationJobZSI.NewTranslationJob_services import *

def studlyCapitalize(s):
    return s[0].capitalize()+s[1:]

loc = NewTranslationJobLocator()
port = loc.getNewTranslationJobPortType("http://localhost:8081/services.py", tracefile=sys.stdout)

req = startTranslationJobRequest()
p = json.load(file("/home/small/code/webservices/translationnode/trialTranslationRequest.js"))
req.Param0 = req.new_param0()
for k, v in p["param0"].items():
    setattr(req.Param0, studlyCapitalize(k), v)

resp = port.startTranslationJob(req)
