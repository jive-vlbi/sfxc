#! /usr/bin/python

import json
import os
import sys
import threading
import time
import urlparse

import jsonrpclib
jsonrpclib.config.version = 1.0
import jsonrpclib.SimpleJSONRPCServer
import web

from decoder import Mark4, Mark5B
import scp
import config

urlparse.uses_relative.append('scp')
urlparse.uses_netloc.append('scp')
urlparse.uses_params.append('scp')
urlparse.clear_cache()

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

os.environ['TZ'] = 'UTC'
time.tzset()

urls = (
    '/data', 'data'
)

app = web.application(urls, globals())

def transfer_data_callback(src, dest, chunk, num_chunks, start, stop,
                           offset, len, notify):
    station = config.input['station']
    path = urlparse.urlparse(dest).path
    path = os.path.join(path, "%s.%d" % (os.path.basename(src), chunk))
    dest = urlparse.urljoin(dest, path)
    scp.scp(src, dest, offset, len)
    if notify:
        s = jsonrpclib.Server(notify, verbose=0)
        print s.transfer_data_completed(station, chunk, num_chunks,
                                        time2vex(start), time2vex(stop), dest)
        pass
    return

def transfer_data_async(station, start, stop, size, dest, notify):
    data = config.input['data']
    format = config.input['format']
    rate = config.input['rate']

    # Minumum chunk size is 4 seconds worth of data.  We round up to
    # that as well.
    secs_per_chunk = max(4, (((size / (rate / 8)) + 3) / 4) * 4)

    type = format.split(':')[0]
    if type == "mark4":
        ntracks = int(format.split(':')[1])
        decoder = Mark4(ntracks)
    else:
        decoder = Mark5B()
        pass

    # Get rid of the data files that have no overlap with the
    # requested time range.
    last_src = None
    for src in sorted(data, key=data.get):
        if vex2time(data[src]) >= stop:
            del data[src]
            continue
        if last_src and vex2time(data[src]) <= start:
            del data[last_src]
            pass
        last_src = src
        continue

    for src in sorted(data, key=data.get):
        decoder.chunk_data(src, start, stop, secs_per_chunk,
                           dest, transfer_data_callback, notify)
        continue
    print "transfer_data_async finished"
    return

def transfer_data(station, start, stop, size, dest, notify):
    # Input validation
    if station.lower() != config.input['station'].lower():
        raise ValueError, "Invalid station '%s'" % station
    start = vex2time(start)
    stop = vex2time(stop)
    if urlparse.urlparse(dest).scheme != 'scp':
        raise ValueError, "Invalid destination URL"

    # Handle the transfer asynchronously by starting a seperate thread
    # to do the actual transfer.
    t = threading.Thread(target=transfer_data_async,
                         args=(station, start, stop, size, dest, notify))
    t.start()
    return "Success"

def echo(msg):
    return msg

dispatcher = jsonrpclib.SimpleJSONRPCServer.SimpleJSONRPCDispatcher()
dispatcher.register_function(transfer_data)
dispatcher.register_function(echo)

class data:
    def POST(self):
        web.header('Content-Type', 'application/json')
        response = dispatcher._marshaled_dispatch(web.data())
        return response

    pass

if __name__ == "__main__":
    ctrl_file = sys.argv[1]
    del sys.argv[1]

    fp = open(ctrl_file)
    config.input = json.load(fp)
    fp.close()

    station = config.input['station']
    format = config.input['format']
    rate = config.input['rate']
    type = format.split(':')[0]
    assert type == "mark4" or type == "mark5b"
    if  type == "mark4":
        ntracks = int(format.split(':')[1])
        assert ntracks == 8 or ntracks == 16 or ntracks == 32 or ntracks == 64
        pass
    print "station %s format %s @ %d Mb/s"% (station, format, rate / 1000000)

    app.run()
    pass
