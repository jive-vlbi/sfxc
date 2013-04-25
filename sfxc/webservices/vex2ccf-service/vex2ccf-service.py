#! /usr/bin/python

import json
import logging
import os
import signal
import sys
import tempfile
import time

import jsonrpclib
jsonrpclib.config.version = 1.0
import jsonrpclib.SimpleJSONRPCServer
import web

# JIVE Python modules
import vex
from vex2ccf import vex2ccf as do_vex2ccf

os.environ['TZ'] = 'UTC'
time.tzset()

urls = (
    '/vex2ccf', 'vex2ccf_service'
)

app = web.application(urls, globals())

def finish(*args):
    logging.critical("Caught a SIGTERM: exiting")
    sys.exit(0)

def restart(*args):
    logging.warning("Caught a SIGHUP")

def daemonize():
    # a double-fork explanation:
    # http://code.activestate.com/recipes/66012-fork-a-daemon-process-on-unix/#c12
    try:
        pid = os.fork()
    except OSError, r:
        die("Failed to fork"+r)
	pid = None
    if (pid > 0):
        # parent exits
        sys.exit(0)
    try:
        os.close(sys.stdin.fileno())
    except OSError: pass
    try:
        os.close(sys.stdin.fileno())
    except OSError: pass
    try:
        os.close(sys.stderr.fileno())
    except OSError: pass
    ## Add dummies for libraries:
    sys.stdin = open('/dev/null', 'r')
    sys.stdout = open('/dev/null', 'w')
    sys.stderr = open('/dev/null', 'w')

    try:
        os.chdir("/")
    except OSError:
        die("Failed to change directory")
    try:
        os.setsid()
    except OSError:
        die("Failed to setsid")
    os.umask(0)
    logging.debug("About to set signals")
    # * Run in the background.
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    signal.signal(signal.SIGHUP, restart)
    signal.signal(signal.SIGTERM, finish)
    try:
        pid2 = os.fork()
    except OSError, r:
        die("Second fork failed: "+r)
    if pid2 > 0:
        # parent returns
        sys.exit(0)

def vex2ccf(s):
    v = vex.parse(s)
    json_output = do_vex2ccf(v)
    return json_output

dispatcher = jsonrpclib.SimpleJSONRPCServer.SimpleJSONRPCDispatcher()
dispatcher.register_function(vex2ccf)

class vex2ccf_service:
    def POST(self):
        print web.data()
        web.header('Access-Control-Allow-Origin', '*')
        web.header('Content-Type', 'application/json')
        response = dispatcher._marshaled_dispatch(web.data())
        print response
        return response

    pass

if __name__ == "__main__":
    daemonize()
    app.run()
    pass
