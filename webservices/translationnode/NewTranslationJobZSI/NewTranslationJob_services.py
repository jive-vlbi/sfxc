################################################## 
# NewTranslationJob_services.py 
# generated by ZSI.generate.wsdl2python
##################################################


from NewTranslationJob_services_types import *
import urlparse, types
from ZSI.TCcompound import ComplexType, Struct
from ZSI import client
import ZSI
from ZSI.generate.pyclass import pyclass_type

# Locator
class NewTranslationJobLocator:
    NewTranslationJobPortType_address = "http://localhost:8080/axis2/services/NewTranslationJob"
    def getNewTranslationJobPortTypeAddress(self):
        return NewTranslationJobLocator.NewTranslationJobPortType_address
    def getNewTranslationJobPortType(self, url=None, **kw):
        return NewTranslationJobSOAP11BindingSOAP(url or NewTranslationJobLocator.NewTranslationJobPortType_address, **kw)

# Methods
class NewTranslationJobSOAP11BindingSOAP:
    def __init__(self, url, **kw):
        kw.setdefault("readerclass", None)
        kw.setdefault("writerclass", None)
        # no resource properties
        self.binding = client.Binding(url=url, **kw)
        # no ws-addressing

    # op: startTranslationJob
    def startTranslationJob(self, request):
        if isinstance(request, startTranslationJobRequest) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="urn:startTranslationJob", **kw)
        #check for soap, assume soap:fault
        if self.binding.IsSOAP(): self.binding.Receive(None, **kw)

startTranslationJobRequest = ns0.startTranslationJob_Dec().pyclass
