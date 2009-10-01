import urllib, httplib
import simplejson as json
import cgi
## Adapted from:
## http://users.skynet.be/pascalbotte/rcx-ws-doc/postxmlpython.htm

soap = open("SOAPs/correlatedData-scp.soap").read()

webservice = httplib.HTTP("huygens.jive.nl", 8081)
webservice.putrequest("POST", "/services.py")
webservice.putheader("Host", "huygens.jive.nl:8081")
webservice.putheader("User-Agent", "Python post")
webservice.putheader("Content-Type", "application/soap+xml; charset=\"UTF-8\"")
webservice.putheader("Content-Length", "%d" % len(soap))
webservice.putheader("action", "urn:startTranslationJob")
webservice.endheaders()
webservice.send(soap)

statuscode, statusmessage, header = webservice.getreply()
print "Response: ", statuscode, statusmessage
print "headers: ", header
res = webservice.getfile().read()
print res
