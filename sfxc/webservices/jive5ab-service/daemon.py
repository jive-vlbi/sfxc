import logging
import os
import signal
import sys

def finish(*args):
    logging.critical("Caught a SIGTERM: exiting")
    raise KeyboardInterrupt

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

