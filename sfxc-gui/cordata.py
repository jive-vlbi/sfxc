#! /usr/bin/python

# Standard Python modules
from datetime import datetime, timedelta
import json
import os
import optparse
import re
import subprocess
import struct
import sys
import time
import urlparse

# Qt and Qwt
from PyQt4 import Qt, QtCore, QtGui
import PyQt4.Qwt5 as Qwt
from PyQt4.Qwt5.anynumpy import *

# NumPy
import numpy as np

# JIVE Python modules
from vex_parser import Vex

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

global_hdr = 'I32sHHIIIIIB3x'
timeslice_hdr = 'IIII'
uvw_hdr = 'II3d'
stat_hdr = 'BBBB4II'
baseline_hdr = 'IBBBx'

class CorrelatedData:
    def __init__(self, vex, output_file):
        self.output_file = output_file
        self.offset = 0

        self.fp = None
        self.integration_slice = 0
        self.integration_time = 0
        self.weights = {}
        self.history = 32
        self.correlations = {}
        self.time = {}
        self.start_time = 0
        self.current_time = 0

        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue
        self.stations.sort()
        return

    def append(self, output_file, offset):
        self.output_file = output_file
        self.offset = offset

        self.fp = None
        return

    def read(self):
        if self.fp == None:
            try:
                self.fp = open(self.output_file, 'r')
            except:
                return

            try:
                h = struct.unpack(global_hdr, self.fp.read(struct.calcsize(global_hdr)))
            except:
                self.fp.close()
                self.fp = None
                return

            exper = h[1].strip('\x00')
            hour = h[4] / 3600
            min = (h[4] % 3600) / 60
            sec = h[4] % 60
            strtime = "%dy%dd%02dh%02dm%02ds" % (h[2], h[3], hour, min, sec)
            self.start_time = vex2time(strtime)

            self.number_channels = h[5]
            self.integration_time = h[6] * 1e-6
            pass

        secs = 0
        pos = self.fp.tell()
        while self.fp:
            try:
                h = struct.unpack(timeslice_hdr, self.fp.read(struct.calcsize(timeslice_hdr)))

                integration_slice = h[0]
                number_baselines = h[1]
                number_uvw_coordinates = h[2]
                number_statistics = h[3]

                slot = integration_slice % self.history
                if integration_slice != self.integration_slice:
                    self.integration_slice = integration_slice
                    pass

                for i in xrange(number_uvw_coordinates):
                    h = struct.unpack(uvw_hdr, self.fp.read(struct.calcsize(uvw_hdr)))
                    continue

                for i in xrange(number_statistics):
                    h = struct.unpack(stat_hdr, self.fp.read(struct.calcsize(stat_hdr)))
                    weight = 1.0 - (float(h[8]) / (h[4] + h[5] + h[6] + h[7] + h[8]))
                    station = self.stations[h[0]]
                    if not station in self.time:
                        self.time[station] = []
                        pass
                    if not station in self.weights:
                        self.weights[station] = {}
                        pass
                    idx = h[1] * 4 + h[2] * 2 + h[3]
                    if not idx in self.weights[station]:
                        self.weights[station][idx] = []
                        pass

                    secs = self.offset + integration_slice * self.integration_time
                    if not secs in self.time[station]:
                        self.time[station].append(secs)
                        self.current_time = self.start_time + integration_slice * self.integration_time
                        pass
                    if len(self.weights[station][idx]) < len(self.time[station]):
                        self.weights[station][idx].append(weight)
                        pass
                    continue

                for i in xrange(number_baselines):
                    h = struct.unpack(baseline_hdr, self.fp.read(struct.calcsize(baseline_hdr)))
                    buf = self.fp.read((self.number_channels + 1) * 8)
                    if not len(buf) == (self.number_channels + 1) * 8:
                        raise Hell
                    idx = h[3]
                    baseline = (self.stations[h[1]], self.stations[h[2]])
                    if not baseline in self.correlations:
                        self.correlations[baseline] = {}
                        pass
                    if not idx in self.correlations[baseline]:
                        self.correlations[baseline][idx] = \
                            np.zeros((self.history, self.number_channels + 1),
                                     dtype=np.complex64)
                        pass
                    self.correlations[baseline][idx][slot] = \
                        np.frombuffer(buf, dtype=np.complex64)
                    continue
                pos = self.fp.tell()
            except:
                self.fp.seek(pos)
                break

            continue

        return

    pass


if __name__ == '__main__':
    usage = "usage: %prog [options] vexfile ctrlfile"
    parser = optparse.OptionParser(usage=usage)

    (options, args) = parser.parse_args()
    if len(args) < 2:
        parser.error("incorrect number of arguments")
        pass

    os.environ['TZ'] = 'UTC'
    time.tzset()

    vex_file = args[0]
    ctrl_files = args[1:]

    vex = Vex(vex_file)

    data = CorrelatedData(vex, ctrl_files[0])
    data.read()

    sys.exit(0)
