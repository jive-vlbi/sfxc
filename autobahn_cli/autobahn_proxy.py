################################################################################
#
# Autobahn_proxy.py
# A python library to control the Autobahn service (see: ).
# The library needs the ZSI web-service framework and pyXML.
# you need to generate the ZSI backend corresponding to the wsdl file.
# cmd:
#    python  wsdl2py --complexType  --url http://..../uap.wdsl
#
# author: Damien Marchal
# date: 11/08/2008
#
################################################################################
import time, datetime
import sys

from ZSI import ServiceProxy
from UserAccessPointService_services import *
import UserAccessPointService_services_types
from UserAccessPointService_services_types import *

class Autobahn_proxy():
  """
  A proxy object to the autobahn services. In addition to the default functions
  exposed by the Autobahn user API it offer additional function like:
  - port_to_name and name_to_port
  - get_useraccess_idm()
  - cancel_request()
  - create_request()
  - add_reservation()
  - submit_request()
  """
  def __init__(self, userName, userEmail, url=None):
    loc = UserAccessPointServiceLocator()
    kw = { 'tracefile' : sys.stdout }
    #self.port=loc.getUserAccessPoint(url=url, **kw)
    self.port=loc.getUserAccessPoint(url=url)
    self.port2name={
        '10.10.32.27' : 'Internet2 B',
        '10.10.32.28' : 'Internet2 A',
        '10.10.32.29' : 'Amsterdam 3901',
        '10.10.32.30' : 'Amsterdam 3902',
        '10.10.32.31' : 'Amsterdam 3903',
        '10.10.32.32' : 'Amsterdam 3904',
        '10.10.32.33' : 'FCCN',
        '10.11.32.6'  : 'Athens',
        '10.11.32.7'  : 'UoC',
        '10.12.32.5'  : 'Dublin 2',
        '10.12.32.6'  : 'Dublin 1',
        '10.13.32.4'  : 'Poznan',
        '10.14.32.2'  : 'Rome',
        '10.16.32.2'  : 'Zagreb'
    }
    self.name2port = {}
    for i in self.port2name.keys():
      self.name2port[ self.port2name[i] ] = i

    # We only know these two IDM. More should come and even may be
    # retreived from a webservice call
    self.list_idm=["http://srv2.lon.uk.geant2.net:8080/autobahn/uap",
                   "http://poznan.autobahn.psnc.pl:8080/autobahn/uap",
                   "http://abcpl.dir.garr.it:8080/autobahn/interdomain",
                   "http://kanga.heanet.ie:8080/autobahn/interdomain",
                   "http://gn2jra3.grnet.gr:8080/autobahn/interdomain",
                   "http://lambda.carnet.hr:8080/autobahn/interdomain"]


    # We stores in params all default paramters for the queries.
    # default capacity is 1Gb
    # default delay is 10000
    # default priority is HIGHEST
    # resiliency is NONE
    # bidirection is TRUE
    self.params={}
    self.params["userName"]=userName
    self.params["userEmail"]=userEmail
    self.params["userDomain"]="Undefined"
    self.params["justifcation"]="None"

    self.params["startPort"]=None
    self.params["stopPort"]= None
    self.params["startTime"]=None
    self.params["stopTime"]= None

    self.params["capacity"]= 1000000000
    self.params["description"]= "No description"
    self.params["maxDelay"]= 10000
    self.params["bidirectional"]= True
    self.params["priority"]= "HIGHEST"
    self.params["resiliency"]= "NONE"

    # compute the ports that can be used as starting port for this IDM
    self.valid_start_ports=self.get_domain_client_ports()

  def get_useraccess_idm(self):
    """
    Return a list of known IDM. Currently the list is hardcoded. Maybe in some
    future the IDM should be retreived from the default IDM.
    """
    return self.list_idm

  def port_to_name(self, port):
    """
    Convert a port number to its identifying string
    """
    if self.port2name.has_key(port):
      return self.port2name[port]
    return None

  def name_to_port(self, name):
    """
    Convert a string id to the corresponding port number
    """
    if self.name2port.has_key(name):
      return self.name2port[name]
    return None

  def get_all_client_ports(self):
    """
    Return all the ports availables in the autobahn infrastructure. An object
    is returned with the result in the attribute named: _Ports
    """
    param = getAllClientPorts()
    return self.port.getAllClientPorts(param)._Ports

  def get_domain_client_ports(self):
    """
    Return all the ports availables on a specific domain. And object
    is returned with the result in the attribute named: _Ports
    """
    param = getDomainClientPorts()
    return self.port.getDomainClientPorts(param)._Ports

  def create_request(self,
                     userName=None,
                     userEmail=None,
                     justification=None):
    """
    Create a new request object. The object is return to the user. If userName,
    userEmail and justification are not provided the default values are used.
    Once a request object is created it is possible to add to him new
    reservations entries using the add_reservation function.
    """
    userinfo = ns0.ServiceRequest_Def(pname="ServiceRequest").pyclass()
    userinfo._userName = userName or self.params["userName"]
    userinfo._userEmail = userEmail or self.params["userEmail"]
    #userinfo._userDomain = userDomain or self.params["userDomain"]
    userinfo._justification = justification or self.jparams["justification"]
    userinfo._reservations = []
    return userinfo

  def set_default_param(self, name, value):
    """
    Set a default value to an request attribute.
    """
    if not self.params.has_key(name):
      raise NameError, "Invalid parameters name: "+name
    self.params[name] = value

  def add_reservation(self,
                      request,
                      startPort=None, stopPort=None,
                      startTime=None, stopTime=None,
                      capacity=None, description=None,
                      maxDelay=None, bidirectional=None,
                      priority=None, resiliency=None):
    """
    A request can contains multiple reservation. Use this function to add a new
    reservation entry to a request. All missing parameters will be initialized
    with the default values. If startTime is not provided nor has default value
    the reservation will be process immediately.
    """
    res = ns0.ReservationRequest_Def(pname="ReservationRequest").pyclass()
    res._startPort = startPort or self.params["startPort"]
    res._endPort = stopPort or self.params["stopPort"]
    res._startTime = startTime or self.params["startTime"]
    res._endTime =  stopTime or self.params["stopTime"]
    res._capacity = capacity or self.params["capacity"]
    if not res._startTime:
      res._processNow = True
      starttime = list(datetime.datetime.now().timetuple())
      starttime[6] = 0
      res._startTime = starttime
    else:
      res._processNow = False
    res._description = description or self.params["description"]
    res._maxDelay = maxDelay or self.params["maxDelay"]
    res._bidirectional = bidirectional or self.params["bidirectional"]
    res._priority = priority or self.params["priority"]
    res._resiliency = resiliency or self.params["resiliency"]

    if res._startPort in self.valid_start_ports:
      request._reservations.append(res)
    else:
      raise NameError, 'Invalid start port: '+res._startPort

  def submit_request(self, request):
    """
    Submit the given request to the IDM. A requestID is returned.
    """
    service = submitService()
    service._request = request
    service._reservations = []
    res = self.port.submitService(service)
    return res._serviceID

  def get_reservations_info(self, requestID):
    """
    retreive the informations relatives to a requestID. A list of
    reservations is returned containing their states.
    """
    qs = queryService()
    qs._serviceID = requestID
    res = self.port.queryService(qs)._ServiceResponse
    return res._reservations

  def cancel_request(self, requestID):
    """
    Cancel a request using its requestID.
    """
    cr = cancelService()
    cr._serviceID = requestID
    self.port.cancelService(cr)
