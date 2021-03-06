##################################################
# UserAccessPointService_services.py
# generated by ZSI.generate.wsdl2python
##################################################


from UserAccessPointService_services_types import *
import urlparse, types
from ZSI.TCcompound import ComplexType, Struct
from ZSI import client
import ZSI
from ZSI.generate.pyclass import pyclass_type

# Locator
class UserAccessPointServiceLocator:
    #UserAccessPoint_address = "http://poznan.autobahn.psnc.pl:8080/autobahn/uap"
    UserAccessPoint_address = "http://srv2.lon.uk.geant2.net:8080/autobahn/uap"
    def getUserAccessPointAddress(self):
        return UserAccessPointServiceLocator.UserAccessPoint_address
    def getUserAccessPoint(self, url=None, **kw):
        return UserAccessPointServiceSoapBindingSOAP(url or UserAccessPointServiceLocator.UserAccessPoint_address, **kw)

# Methods
class UserAccessPointServiceSoapBindingSOAP:
    def __init__(self, url, **kw):
        kw.setdefault("readerclass", None)
        kw.setdefault("writerclass", None)
        # no resource properties
        self.binding = client.Binding(url=url, **kw)
        # no ws-addressing

    # op: getDomainClientPorts
    def getDomainClientPorts(self, request):
        if isinstance(request, getDomainClientPorts) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(getDomainClientPortsResponse.typecode)
        return response

    # op: submitService
    def submitService(self, request):
        if isinstance(request, submitService) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(submitServiceResponse.typecode)
        return response

    # op: modifyReservation
    def modifyReservation(self, request):
        if isinstance(request, modifyReservation) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(modifyReservationResponse.typecode)
        return response

    # op: cancelService
    def cancelService(self, request):
        if isinstance(request, cancelService) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(cancelServiceResponse.typecode)
        return response

    # op: getAllClientPorts
    def getAllClientPorts(self, request):
        if isinstance(request, getAllClientPorts) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(getAllClientPortsResponse.typecode)
        return response

    # op: queryService
    def queryService(self, request):
        if isinstance(request, queryService) is False:
            raise TypeError, "%s incorrect request type" % (request.__class__)
        kw = {}
        # no input wsaction
        self.binding.Send(None, None, request, soapaction="", **kw)
        # no output wsaction
        response = self.binding.Receive(queryServiceResponse.typecode)
        return response

getDomainClientPorts = ns1.getDomainClientPorts_Dec().pyclass

getDomainClientPortsResponse = ns1.getDomainClientPortsResponse_Dec().pyclass

submitService = ns1.submitService_Dec().pyclass

submitServiceResponse = ns1.submitServiceResponse_Dec().pyclass

modifyReservation = ns1.modifyReservation_Dec().pyclass

modifyReservationResponse = ns1.modifyReservationResponse_Dec().pyclass

cancelService = ns1.cancelService_Dec().pyclass

cancelServiceResponse = ns1.cancelServiceResponse_Dec().pyclass

getAllClientPorts = ns1.getAllClientPorts_Dec().pyclass

getAllClientPortsResponse = ns1.getAllClientPortsResponse_Dec().pyclass

queryService = ns1.queryService_Dec().pyclass

queryServiceResponse = ns1.queryServiceResponse_Dec().pyclass
