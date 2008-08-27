#!/usr/bin/env python

################################################################################
#
# Autobahn_test.py
# Little file to command line utility around the autobahn_proxy. Can
# create/cancel a path between src and destination using a specific IDM.
# Can poll to print the status of a path. Can print the list of ports on a
# domain and total list of port.
# try it with -h to print the possible arguments.
# depends on:
#    Autobahn_proxy
#    ZSI (SOAP webservice framework for python)
#    using ZSI and wsdl file the backend has to be generated using the command:
#    python  wsdl2py --complexType --url url_to_the_autobahn_service_wsdl
#
# author: Damien Marchal
# date: 11/08/2008
#
################################################################################

import time, datetime
import sys, getopt
import autobahn_proxy
from autobahn_proxy import *
from optparse import OptionParser

def main():
    requestID = None
    default_username = "dmarchal"
    default_useremail = "dmarchal@science.uva.nl"
    default_idm = "http://srv2.lon.uk.geant2.net:8080/autobahn/uap"

    parser = OptionParser()
    parser.add_option("-s", "--submit", nargs=1, dest="create", help="Create a new reservation")
    parser.add_option("-r", "--requestid", dest="requestID", help="Specifiy a specific requestID")
    parser.add_option("-c", "--cancel", dest="cancel", action="store_true", help="Cancel an existing request")
    parser.add_option("-i", "--info", action="store_true", dest="info", help="Get the info about an existing request")
    parser.add_option("-p", "--poll", action="store_true", dest="loop", help="Infinite loop printing the info about the requestID")
    parser.add_option("-l", "--list-port", action="store_true", dest="listport", help="Print the list of port available on the IDM")
    parser.add_option("-L", "--list-all-port", action="store_true", dest="listallport", help="Print the list of all ports on the IDM")
    parser.add_option("-D", "--list-domain", action="store_true", dest="listdomain", help="Print the list of domain available")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", help="Add more information to the output.")
    parser.add_option("-d", "--domain", dest="domain", help="Specify a idm to use, default=["+default_idm+"]")
    parser.add_option("-u", "--username", dest="username", help="Specify a username, default=["+default_username+"]" )
    parser.add_option("-e", "--useremail", dest="useremail", help="Specify a usermeail, default=["+default_useremail+"]")

    (options, args) = parser.parse_args()

    if len(sys.argv)<=1:
      parser.error("Missing argument, try with -h")

    if options.domain:
      default_idm = options.domain

    if options.username:
      default_username=options.username

    if options.useremail:
      default_useremail=options.useremail

    if options.verbose:
      print "user name: "+default_username
      print "user email: "+default_useremail
      print "domain: "+default_idm

    # Create a proxy to the autobahn service, all the parameters
    # are optional.
    proxy = Autobahn_proxy(
              userName = default_username,
              userEmail= default_useremail,
              url=default_idm
            )

    #print "Options: "+`options`
    if not len(args) == 0:
      parser.error("invalid option:"+`args`)

    if options.requestID:
      requestID = options.requestID

    # Do we want to print the port of a domain
    if options.listdomain:
      # Retreive the port that can be used on this domain
      print "Available domains:"
      ports = proxy.get_useraccess_idm()
      print "\t* "+ports[0]
      for i in ports[1:]:
        print "\t  "+i

    # Do we want to print the port of a domain
    if options.listport:
      # Retreive the port that can be used on this domain
      print "Listing source ports:"
      ports = proxy.get_domain_client_ports()
      for i in ports:
        print "   Ports "+i+" :"+proxy.port_to_name(i)

    # Do we want to print the port of a domain
    if options.listallport:
      # Retreive the port that can be used on this domain
      print "Listing all ports:"
      ports = proxy.get_all_client_ports()
      for i in ports:
        print "   Ports "+i+" :"+proxy.port_to_name(i)

    # Retreive the current data/time and put it in a list to make
    # ZSI happy and able to serizalize it in SOAP/XML
    starttime = list(datetime.datetime.now().timetuple())
    starttime[6] = 0

    # start the service in one hour from now
    starttime[3] = starttime[3]+1

    # stop the service in 2 hours from now
    stoptime = list(datetime.datetime.now().timetuple())
    stoptime[3] = stoptime[3]+2

    if options.create:
      # configure the default parameters to re-use when creating a
      # reservation with the add_reservation function.
      # this means that all the services created will use the same
      # start/stop time without resiliency
      # proxy.set_default_param("startTime", starttime)
      proxy.set_default_param("stopTime", stoptime)
      proxy.set_default_param("resiliency", "NONE")

      # what happens if the start port is invalid ?
      try:
        # Let's now create a request for service
        # userName and email are optional, here taken from the proxy.
        request = proxy.create_request(justification="Path between:"+options.create)

        dests = options.create.split(',')
        for apair in dests:
          pair = apair.split('-')
          if len(pair) == 2:
            startPort = proxy.name_to_port( pair[0].lstrip() ) or pair[0].lstrip() or None
            stopPort = proxy.name_to_port( pair[1].lstrip() ) or pair[1].lstrip() or None

            # adding a reservation to the request given in first parameters, all the
            # non specified parameters will used the default values.
            proxy.add_reservation(request,
                          startPort=startPort,
                          stopPort=stopPort,
                          maxDelay=200)
          else:
            raise NameError, "Invalid source-destination"

        # fire up the request for service
        requestID = proxy.submit_request(request)
        print "A service has been submitted: "+requestID
      except NameError, msg:
        print "", msg
        print "src-dest:", options.create
        print "    Try to run with -l to list the available ports."
        sys.exit(-1)

    if not( options.cancel or options.info or options.loop):
        sys.exit(0)

    if not requestID:
      parser.error("the options -c, -i and -l requires a valid requestID. They must be used with either -r or -s")

    if options.cancel:
      print "Cancelling: "+requestID
      proxy.cancel_request(requestID)

    if options.info and requestID:
      # retreive the reservations info using the requestID
      reservations = proxy.get_reservations_info(requestID)

      # print the status for each of the reservation in a service request
      for res in reservations:
        print "Status is: "+res.State

    if options.loop:
      # let's loop infinitely
      while True:
        # wait one second
        time.sleep(1)

        # retreive the reservations info using the requestID
        reservations = proxy.get_reservations_info(requestID)
        # print the status for each of the reservation in a service request
        for res in reservations:
          print "Status is: "+res.State

# Ok I'm a bit lazy to do this big try-catch,
try:
  main()
except ZSI.FaultException, msg:
  print "Autobahn web-service exception: ", msg
