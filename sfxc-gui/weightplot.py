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
from vex import Vex
from cordata import CorrelatedData

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
    color = [ "#bae4b3", "#74c476", "#31a354", "#006d2c",
              "#bdd7e7", "#6baed6", "#3182bd", "#08519c",
              "#fcae91", "#fb6a4a", "#de2d26", "#a50f15",
              "#cccccc", "#999999", "#666666", "#000000" ]

    def __init__(self, parent, station, start, stop, gaps, scans, history, *args):
        Qwt.QwtPlot.__init__(self, *args)

        self.start = start
        self.stop = stop
        self.history = history

        self.setCanvasBackground(Qt.Qt.white)

        self.x = []
        self.y = {}
        self.station = station

        scaleDraw = ScaleDraw(start);
        self.setAxisScaleDraw(Qwt.QwtPlot.xBottom, scaleDraw)
        self.scroll(0)

        self.setAxisTitle(Qwt.QwtPlot.yLeft, station)
        self.setAxisScale(Qwt.QwtPlot.yLeft, 0, 1.0)
        self.setAxisMaxMajor(Qwt.QwtPlot.yLeft, 2)
        self.curve = {}

        self.scans = scans
        self.labels = []
        for scan in scans:
            label = Qwt.QwtPlotMarker()
            label.setValue(scan[0], 0.66)
            text = Qwt.QwtText(scan[1])
            text.setColor(Qt.QColor("gray"))
            text.setPaintAttribute(Qwt.QwtText.PaintUsingTextColor)
            label.setLabel(text)
            label.setLabelAlignment(Qt.Qt.AlignRight)
            label.attach(self)
            self.labels.append(label)
            continue

        self.gaps = gaps
        self.gapcurve = GapCurve()
        x = []
        y = []
        for gap in gaps:
            x.append(gap[0])
            x.append(gap[1])
            y.append(-1)
            y.append(2)
            continue
        self.gapcurve.setData(x, y)
        self.gapcurve.attach(self)

        self.connect(self, Qt.SIGNAL("legendChecked(QwtPlotItem*,bool)"),
                     self.toggleCurve)
        self.parent = parent
        return

    def scroll(self, val):
        start = self.start
        stop = self.stop

        seconds = stop - start
        if seconds > self.history:
            start = self.start + (val * (seconds - self.history)) / 100
            stop = start + self.history
            if stop > self.stop:
                stop = self.stop
                pass
            pass

        seconds = round(stop - start)
        offset = start - self.start
        mins = round(float(stop - start) / (5 * 60))
        mins = max(mins, 1)

        start = datetime.utcfromtimestamp(self.start)
        stop = datetime.utcfromtimestamp(self.stop)
        rounded_time = start.replace(second=0)
        ticks = []
        while rounded_time <= stop - (timedelta(minutes=mins) / 4):
            if rounded_time >= start:
                ticks.append((rounded_time - start).seconds)
                pass
            rounded_time += timedelta(minutes=mins)
            continue
        scaleDiv = Qwt.QwtScaleDiv(offset, offset + seconds, [],[], ticks);
        self.setAxisScaleDiv(Qwt.QwtPlot.xBottom, scaleDiv)
        return

    def resizeEvent(self, e):
        Qwt.QwtPlot.resizeEvent(self, e)

        if self.labels:
            map = self.canvasMap(Qwt.QwtPlot.xBottom)

            x1 = map.transform(0)
            scan = 0
            for gap in self.gaps:
                x2 = map.transform(gap[0])
                width = x2 - x1
                if self.labels[scan].label().textSize().width() >= width - 4:
                    self.labels[scan].hide()
                else:
                    self.labels[scan].show()
                    pass
                x1 = map.transform(gap[1])
                scan += 1
                continue
            pass
            
        return

    def toggleCurve(self, curve, state):
        for idx in self.curve:
            if self.curve[idx] == curve:
                break
            continue
            
        for station in self.parent.plot:
            try:
                self.parent.plot[station].curve[idx].setVisible(not state)
                self.parent.plot[station].replot()
            except:
                pass
            continue
        return

    pass


class WeightPlotWindow(Qt.QWidget):
    def __init__(self, vex, ctrl_files, cordata, realtime, history=3600, *args):
        Qt.QWidget.__init__(self, *args)

        exper = vex['GLOBAL']['EXPER']
        exper = vex['EXPER'][exper]['exper_name']
        self.setWindowTitle(exper + " Weights")

        self.integration_slice = -1
        self.realtime = realtime
        self.history = history

        self.output_file = 0
        self.output_files = []
        self.offsets = []
        stations = []
        for ctrl_file in ctrl_files:
            fp = open(ctrl_file, 'r')
            json_input = json.load(fp)
            fp.close()

            output_file = urlparse.urlparse(json_input['output_file']).path
            try:
                if json_input['multi_phase_center']:
                    source = None
                    start = vex2time(json_input['start'])
                    for scan in vex['SCHED']:
                        if start >= vex2time(vex['SCHED'][scan]['start']):
                            source = vex['SCHED'][scan]['source']
                            pass
                        continue
                    if source:
                        output_file = output_file + '_' + source
                        pass
                    pass
            except:
                pass
            try:
                if json_input['pulsar_binning']:
                    output_file = output_file + '.bin1'
                    pass
            except:
                pass
            self.output_files.append(output_file)

            # If the start time is specified as "now" we'll need to
            # look in the correlator output to fine the real start
            # time of the job.  If the correlator output file doesn't
            # exist yet, wait until one shows up.
            while json_input['start'] == "now":
                try:
                    fp = open(output_file, 'r')
                except:
                    pass
                try:
                    h = struct.unpack(global_hdr,
                                      fp.read(struct.calcsize(global_hdr)))
                    hour = h[4] / 3600
                    min = (h[4] % 3600) / 60
                    sec = h[4] % 60
                    json_input['start'] = "%dy%dd%02dh%02dm%02ds" \
                        % (h[2], h[3], hour, min, sec)
                except:
                    time.sleep(1)
                    fp.close()
                    pass
                continue

            # If the stop time is specified as "end", determine the
            # actual stop time from the VEX file.
            if json_input["stop"] == "end":
                json_input["stop"] = vex['EXPER'][exper]['exper_nominal_stop']
                pass

            start = vex2time(json_input['start'])
            stop = vex2time(json_input['stop'])
            for station in json_input['stations']:
                if station not in stations:
                    stations.append(station)
                    pass
                continue
            self.offsets.append(start)
            continue

        start = self.offsets[0]

        gaps = []
        scans = []
        prev_stop_time = None
        for scan in vex['SCHED']:
            # Loop over all the "station" parameters in the scan, figuring out
            # the real length of the scan.
            start_time = stop_time = 0
            for transfer in vex['SCHED'][scan].getall('station'):
                station = transfer[0]
                stop_time = max(stop_time, int(transfer[2].split()[0]))
                continue

            # Figure out the real start and stop time.
            start_time += vex2time(vex['SCHED'][scan]['start'])
            stop_time += vex2time(vex['SCHED'][scan]['start'])
            if start_time < start and stop_time >= start:
                scans.append((start, scan))
                prev_stop_time = stop_time
                pass
            if start_time >= start and start_time < stop:
                scans.append((start_time, scan))
                if prev_stop_time:
                    gaps.append((prev_stop_time, start_time))
                    pass
                prev_stop_time = stop_time
                pass
            continue
        gaps.append((stop, stop))

        for i in xrange(len(self.offsets)):
            self.offsets[i] -= start
            continue

        for i in xrange(len(gaps)):
            gaps[i] = (gaps[i][0] - start, gaps[i][1] - start)
            continue

        for i in xrange(len(scans)):
            scans[i] = (scans[i][0] - start, scans[i][1])
            continue

        self.start = start
        self.stop = stop

        self.fp = None

        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue
        self.stations.sort()

        self.plot = {}
        self.layout = Qt.QGridLayout()
        for station in stations:
            self.plot[station] = WeightPlot(self, station, start, stop, gaps, scans, history)
            self.layout.addWidget(self.plot[station])
            lastplot = self.plot[station]
            lastplot.enableAxis(Qwt.QwtPlot.xBottom, False)
            self.layout.setRowStretch(self.layout.rowCount() - 1, 100)
            self.last_station = station
            scans = []
            continue
        legend = Qwt.QwtLegend()
        legend.setItemMode(Qwt.QwtLegend.CheckableItem)
        lastplot.insertLegend(legend, Qwt.QwtPlot.ExternalLegend)

        self.scroll = Qt.QScrollBar(Qt.Qt.Horizontal)
        self.scroll.valueChanged.connect(self.scrollPlots)

        self.box = Qt.QVBoxLayout(self)
        self.box.addLayout(self.layout)
        self.box.addWidget(lastplot.legend())
        if not self.realtime and (stop - start) > self.history:
            self.box.addWidget(self.scroll)

        if cordata:
            self.cordata = cordata
        else:
            self.cordata = CorrelatedData(vex, self.output_files[self.output_file], self.realtime)
            pass
        self.output_file += 1

        self.startTimer(500)
        self.resize(1000, len(stations) * 100 + 50)
        pass

    def scrollPlots(self, val):
        for station in self.plot:
            self.plot[station].scroll(val)
            continue
        for station in self.plot:
            self.plot[station].replot()
            continue
        pass

    def stretch(self):
        self.plot[self.last_station].enableAxis(Qwt.QwtPlot.xBottom, True)
        self.plot[self.last_station].gapcurve.setItemAttribute(Qwt.QwtPlotItem.Legend, False)
        height = self.plot[self.last_station].height()
        canvasHeight = self.plot[self.last_station].plotLayout().canvasRect().height()
        fixedHeight = height - canvasHeight
        if fixedHeight > 0:
            height = self.layout.contentsRect().height()
            height -= (len(self.plot) - 1) * self.layout.verticalSpacing()
            height /= len(self.plot)
            if height > 0:
                stretch = (height + fixedHeight) * 110 / height
                self.layout.setRowStretch(self.layout.rowCount() - 1, stretch)
                pass
            pass
        return

    def resizeEvent(self, e):
        self.stretch()
        Qt.QWidget.resizeEvent(self, e)
        pass

    def replot(self):
        time = self.cordata.time
        weights = self.cordata.weights
        now = max(self.cordata.current_time - self.start - 0.7 * self.history, 0)
        seconds = max(self.stop - self.start - 0.7 * self.history, 1)
        self.scroll.setValue(100 * now / seconds)
        for station in weights:
            plot = self.plot[station]
            for idx in weights[station]:
                if not idx in plot.curve:
                    title = "SB%d" % (idx / 2)
                    if idx % 2 == 0:
                        title += " R"
                    else:
                        title += " L"
                        pass

                    pen = Qt.QPen()
                    pen.setColor(Qt.QColor(plot.color[idx % 16]))
                    pen.setWidth(3)

                    plot.curve[idx] = Qwt.QwtPlotCurve(title)
                    plot.curve[idx].setData(time[station], weights[station][idx])
                    plot.curve[idx].setPen(pen)
                    plot.curve[idx].setStyle(Qwt.QwtPlotCurve.Dots)
                    plot.curve[idx].attach(plot)
                    if station == self.last_station:
                        self.stretch()
                        pass

                    # Sort curves by detaching them all and
                    # reattach them in the right order.
                    for i in plot.curve:
                        plot.curve[i].detach()
                        plot.curve[i].attach(plot)
                        continue
                    pass

                plot.curve[idx].setData(time[station], weights[station][idx])
                continue
            plot.replot()
            continue
        return

    def timerEvent(self, e):
        self.cordata.read()
        if self.cordata.integration_slice > self.integration_slice:
            self.integration_slice = self.cordata.integration_slice
            self.replot()
        elif not self.realtime:
            if self.output_file < len(self.output_files):
                self.cordata.append(self.output_files[self.output_file], self.offsets[self.output_file])
                self.output_file += 1
                self.integration_slice = 0
                pass
            pass
        return

    pass


if __name__ == '__main__':
    usage = "usage: %prog [options] vexfile ctrlfile"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-H", "--history", dest="history",
                      default=86400, type="int",
                      help="History", metavar="SECONDS")

    (options, args) = parser.parse_args()
    if len(args) < 2:
        parser.error("incorrect number of arguments")
        pass

    os.environ['TZ'] = 'UTC'
    time.tzset()

    vex_file = args[0]
    ctrl_files = args[1:]

    app = QtGui.QApplication(sys.argv)

    vex = Vex(vex_file)

    plot = WeightPlotWindow(vex, ctrl_files, None, False, options.history)
    plot.show()

    sys.exit(app.exec_())
