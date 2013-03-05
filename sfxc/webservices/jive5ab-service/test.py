#! /usr/bin/python

import httplib
import json
import sys
import time

data = {}

fp = open(sys.argv[1])
vex = fp.read()
fp.close()

data['vex'] = vex
data['mtu'] = 9000
data['destination'] = { 'CH01': "sfxc-h0:9999",
                        'CH02': "sfxc-h0:9999" }
#data['port'] = 9999
#data['destination'] = "mk5:///tmp/mk5-Wb"

conn = httplib.HTTPConnection("0.0.0.0:8080")
conn.request("POST", "/configure", json.dumps(data))
resp = conn.getresponse()
print resp.status, resp.reason
print resp.getheaders()
print resp.read()
conn.close()

time.sleep(5)

conn = httplib.HTTPConnection("0.0.0.0:8080")
conn.request("POST", "/start", json.dumps(data))
resp = conn.getresponse()
print resp.status, resp.reason
print resp.getheaders()
print resp.read()
conn.close()

time.sleep(5)

conn = httplib.HTTPConnection("0.0.0.0:8080")
conn.request("POST", "/status", json.dumps(data))
resp = conn.getresponse()
print resp.status, resp.reason
print resp.getheaders()
print resp.read()
conn.close()

time.sleep(5)

conn = httplib.HTTPConnection("0.0.0.0:8080")
conn.request("POST", "/stop", json.dumps(data))
resp = conn.getresponse()
print resp.status, resp.reason
print resp.getheaders()
print resp.read()
conn.close()
