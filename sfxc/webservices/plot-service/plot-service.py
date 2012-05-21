#! /usr/bin/python

import json
import os
import sys
import threading
import time

import jsonrpclib
jsonrpclib.config.version = 1.0
import jsonrpclib.SimpleJSONRPCServer
import web

# NumPy
import numpy as np

# JIVE Python modules
from vex_parser import Vex
from cordata import CorrelatedData

import config

os.environ['TZ'] = 'UTC'
time.tzset()

urls = (
    '/data', 'plot_service'
)

app = web.application(urls, globals())

def fringe(station):
    cordata = config.cordata
    correlations = cordata.correlations

    data = {}
    for baseline in correlations:
        if not station in baseline:
            continue

        if station == baseline[0]:
            key = baseline[1]
        else:
            key = baseline[0]
            pass

        data[key] = {}
        for idx in correlations[baseline]:
            a = correlations[baseline][idx].sum(axis=0)
            if station == baseline[0]:
                a = np.conj(a)
                pass
            b = np.fft.fft(a, cordata.number_channels)
            c = b[(cordata.number_channels / 2):]
            d = b[0:(cordata.number_channels / 2)]
            e = np.concatenate((c, d))
            f = np.absolute(e)
            g = f / np.sum(f)
            data[key][idx] = g.tolist()
            continue
        continue

    return data

dispatcher = jsonrpclib.SimpleJSONRPCServer.SimpleJSONRPCDispatcher()
dispatcher.register_function(fringe)

def read_data(cordata):
    while True:
        cordata.read()
        time.sleep(1)
        continue

class plot_service:
    def POST(self):
        web.header('Access-Control-Allow-Origin', '*')
        web.header('Content-Type', 'application/json')
        response = dispatcher._marshaled_dispatch(web.data())
        return response

    pass

if __name__ == "__main__":
    vex_file = sys.argv[1]
    output_file = sys.argv[2]
    del sys.argv[1]
    del sys.argv[1]

    vex = Vex(vex_file)
    cordata = CorrelatedData(vex, output_file)
    config.cordata = cordata
    t = threading.Thread(target=read_data, args=(cordata,))
    t.start()
    app.run()
    pass
