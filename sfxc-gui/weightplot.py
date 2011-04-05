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

class ScaleDraw(Qwt.QwtScaleDraw):
    def __init__(self, start, *args):
        self.start = start
        Qwt.QwtScaleDraw.__init__(self, *args)
        pass

    def label(self, val):
        tupletime = time.gmtime(self.start + val)
        return Qwt.QwtText(time.strftime("%Hh%Mm", tupletime))
    
    def drawLabel(self, p, val):
        Qwt.QwtScaleDraw.drawLabel(self, p, val)
        pass
    pass

class GapCurve(Qwt.QwtPlotCurve):
    def __init__(self, *args):
        Qwt.QwtPlotCurve.__init__(self, *args)
        pass

    def drawFromTo(self, painter, xMap, yMap, start, stop):
        painter.setPen(Qt.QPen(Qt.Qt.lightGray))
        painter.setBrush(Qt.QBrush(Qt.Qt.FDiagPattern))
        if stop == -1:
            stop = self.dataSize()
            pass
        # force 'start' and 'stop' to be even and positive
        if start & 1:
            start -= 1
            pass
        if stop & 1:
            stop -= 1
            pass
        start = max(start, 0)
        stop = max(stop, 0)
        for i in range(start, stop, 2):
            px1 = xMap.transform(self.x(i))
            py1 = yMap.transform(self.y(i))
            px2 = xMap.transform(self.x(i+1))
            py2 = yMap.transform(self.y(i+1))
            painter.drawRect(px1, py1, (px2 - px1), (py2 - py1))
            continue
        pass
    pass

class WeightPlot(Qwt.QwtPlot):
    color = [ "#ffbf7f", "#ff7f00", "#ffff9d", "#ffff32",
              "#b2ff8c", "#32ff00", "#a5edff", "#19b2ff",
              "#ccbfff", "#654cff", "#ff99bf", "#e51932",
              "#cccccc", "#999999", "#666666", "#000000" ]
    #color = [ "#000000", "#000000", "#000000", "#000000",
    #          "#000000", "#000000", "#000000", "#000000",
    #          "#000000", "#000000", "#000000", "#000000",
    #          "#000000", "#000000", "#000000", "#000000" ]

    def __init__(self, station, start, stop, gaps, *args):
        Qwt.QwtPlot.__init__(self, *args)

        seconds = round(stop - start)
        mins = round(float(stop - start) / (5 * 60))

        self.setCanvasBackground(Qt.Qt.white)

        self.x = []
        self.y = {}
        self.station = station

        scaleDraw = ScaleDraw(start);
        self.setAxisScaleDraw(Qwt.QwtPlot.xBottom, scaleDraw)
        start = datetime.utcfromtimestamp(start)
        stop = datetime.utcfromtimestamp(stop)
        rounded_time = start.replace(second=0)
        ticks = []
        while rounded_time <= stop - (timedelta(minutes=mins) / 4):
            if rounded_time >= start:
                ticks.append((rounded_time - start).seconds)
                pass
            rounded_time += timedelta(minutes=mins)
            continue
        scaleDiv = Qwt.QwtScaleDiv(0, seconds, [],[], ticks);
        self.setAxisScaleDiv(Qwt.QwtPlot.xBottom, scaleDiv)
        self.setAxisTitle(Qwt.QwtPlot.yLeft, station)
        self.setAxisScale(Qwt.QwtPlot.yLeft, 0, 1.0)
        self.setAxisMaxMajor(Qwt.QwtPlot.yLeft, 2)
        self.curve = {}

        curve = GapCurve()
        x = []
        y = []
        for gap in gaps:
            x.append(gap[0])
            x.append(gap[1])
            y.append(-1)
            y.append(2)
            continue
        curve.setData(x, y)
        curve.attach(self)
        return

    pass


class WeightPlotWindow(Qt.QWidget):
    def __init__(self, vex, ctrl_files, *args):
        Qt.QWidget.__init__(self, *args)

        self.setWindowTitle("Weights")

        self.integration_slice = 0

        self.output_file = 0
        self.output_files = []
        self.offsets = []
        gaps = []
        stations = []
        prev_stop = None
        for ctrl_file in ctrl_files:
            fp = open(ctrl_file, 'r')
            json_input = json.load(fp)
            fp.close()
            start = vex2time(json_input['start'])
            stop = vex2time(json_input['stop'])
            for station in json_input['stations']:
                if station not in stations:
                    stations.append(station)
                    pass
                continue
            output_file = urlparse.urlparse(json_input['output_file']).path
            try:
                if json_input['pulsar_binning']:
                    output_file = output_file + '.bin0'
                    pass
            except:
                pass
            self.output_files.append(output_file)
            self.offsets.append(start)
            if prev_stop:
                gaps.append((prev_stop, start))
            prev_stop = stop
            continue

        fp = open(ctrl_files[0], 'r')
        json_input = json.load(fp)
        fp.close()
        start = vex2time(json_input['start'])

        for i in xrange(len(self.offsets)):
            self.offsets[i] -= start
            continue

        for i in xrange(len(gaps)):
            gaps[i] = (gaps[i][0] - start, gaps[i][1] - start)
            continue

        self.fp = None
        self.file_size = 0

        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue
        self.stations.sort()

        self.plot = {}
        self.layout = Qt.QGridLayout(self)
        for station in stations:
            self.plot[station] = WeightPlot(station, start, stop, gaps)
            self.layout.addWidget(self.plot[station])
            lastplot = self.plot[station]
            lastplot.enableAxis(Qwt.QwtPlot.xBottom, False)
            self.layout.setRowStretch(self.layout.rowCount() - 1, 100)
            self.last_station = station
            continue
        lastplot.enableAxis(Qwt.QwtPlot.xBottom, True)
        lastplot.insertLegend(Qwt.QwtLegend(), Qwt.QwtPlot.BottomLegend)
        lastplot.legend().clear()

        self.layout.setRowStretch(self.layout.rowCount() - 1, 200)

        self.startTimer(500)
        self.resize(500, len(stations) * 100 + 50)
        pass

    def resizeEvent(self, e):
        height = self.plot[self.last_station].height()
        canvasHeight = self.plot[self.last_station].plotLayout().canvasRect().height()
        fixedHeight = height - canvasHeight
        if fixedHeight > 0:
            height = self.layout.contentsRect().height()
            height -= (len(self.plot) - 1) * self.layout.verticalSpacing()
            height /= len(self.plot)
            stretch = (height + fixedHeight) * 110 / height
            self.layout.setRowStretch(self.layout.rowCount() - 1, stretch)
            
        Qt.QWidget.resizeEvent(self, e)
        pass

    def timerEvent(self, e):
        if self.fp == None:
            try:
                self.fp = open(self.output_files[self.output_file], 'r')
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
            secs = vex2time("%dy%dd%02dh%02dm%02ds" % (h[2], h[3], hour, min, sec))
            self.number_channels = h[5]
            self.integration_time = h[6] * 1e-6
            pass

        pos = self.fp.tell()
        while self.fp:
            try:
                h = struct.unpack(timeslice_hdr, self.fp.read(struct.calcsize(timeslice_hdr)))

                integration_slice = h[0]
                number_baselines = h[1]
                number_uvw_coordinates = h[2]
                number_statistics = h[3]

                if integration_slice != self.integration_slice:
                    self.integration_slice = integration_slice
                    #for station in self.plot:
                    #    plot = self.plot[station]
                    #    for idx in plot.curve:
                    #        plot.curve[idx].attach(plot)
                    #        plot.curve[idx].setData(plot.x, plot.y[idx])
                    #        continue
                    #    plot.replot()
                    #    continue
                    pass

                for i in xrange(number_uvw_coordinates):
                    h = struct.unpack(uvw_hdr, self.fp.read(struct.calcsize(uvw_hdr)))
                    continue

                for i in xrange(number_statistics):
                    h = struct.unpack(stat_hdr, self.fp.read(struct.calcsize(stat_hdr)))
                    weight = 1.0 - (float(h[8]) / (h[4] + h[5] + h[6] + h[7] + h[8]))
                    station = self.stations[h[0]]
                    plot = self.plot[self.stations[h[0]]]
                    idx = h[1] * 4 + h[2] * 2 + h[3]
                    if not idx in plot.y:
                        plot.y[idx] = []
                        title = "SB%d" % (h[1] * 2 + h[2])
                        if h[3] == 0:
                            title += " R"
                        else:
                            title += " L"
                            pass
                            
                        plot.curve[idx] = Qwt.QwtPlotCurve(title)
                        plot.curve[idx].setData(plot.x, plot.y[idx])
                        plot.curve[idx].setPen(Qt.QPen(Qt.QColor(plot.color[idx % 16])))
                        plot.curve[idx].setStyle(Qwt.QwtPlotCurve.Dots)
                        pass

                    secs = self.offsets[self.output_file] + integration_slice * self.integration_time
                    if not secs in plot.x:
                        plot.x.append(secs)
                        pass
                    plot.y[idx].append(weight)
                    continue

                for i in xrange(number_baselines):
                    h = struct.unpack(baseline_hdr, self.fp.read(struct.calcsize(baseline_hdr)))
                    buf = self.fp.read((self.number_channels + 1) * 8)
                    if not len(buf) == (self.number_channels + 1) * 8:
                        raise Hell
                    continue
                pos = self.fp.tell()
            except:
                self.fp.seek(pos)
                if self.output_file < len(self.output_files):
                    self.fp = None
                    self.output_file += 1
                    pass
                break

            continue

        for station in self.plot:
            plot = self.plot[station]
            for idx in plot.curve:
                plot.curve[idx].attach(plot)
                plot.curve[idx].setData(plot.x, plot.y[idx])
                continue
            plot.replot()
            continue
        pass

    pass


usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)

(options, args) = parser.parse_args()
if len(args) < 2:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]
ctrl_files = args[1:]

app = QtGui.QApplication(sys.argv)

vex = Vex(vex_file)

plot = WeightPlotWindow(vex, ctrl_files)
plot.show()

sys.exit(app.exec_())
