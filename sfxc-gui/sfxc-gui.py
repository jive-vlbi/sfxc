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

# MySQL
import MySQLdb as db

# UI
from progress import *

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

class WeightPlot(Qwt.QwtPlot):
    color = [ "#ffbf7f", "#ff7f00", "#ffff9d", "#ffff32",
              "#b2ff8c", "#32ff00", "#a5edff", "#19b2ff",
              "#ccbfff", "#654cff", "#ff99bf", "#e51932",
              "#cccccc", "#999999", "#666666", "#000000" ]

    def __init__(self, station, start, stop, *args):
        Qwt.QwtPlot.__init__(self, *args)

        seconds = round(stop - start)
        mins = round(float(stop - start) / (5 * 60))
        mins = max(mins, 1)

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

        return

    pass


class WeightPlotWindow(Qt.QWidget):
    def __init__(self, vex, json_input, *args):
        Qt.QWidget.__init__(self, *args)

        self.setWindowTitle("Weights")

        self.integration_slice = 0

        self.fp = None
        self.output_file = urlparse.urlparse(json_input['output_file']).path
        self.file_size = 0

        start = vex2time(json_input['start'])
        stop = vex2time(json_input['stop'])
        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue
        self.stations.sort()

        self.plot = {}
        self.layout = Qt.QGridLayout(self)
        for station in json_input['stations']:
            self.plot[station] = WeightPlot(station, start, stop)
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
        self.resize(500, len(json_input['stations'] * 150))
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
                    for station in self.plot:
                        plot = self.plot[station]
                        for idx in plot.curve:
                            plot.curve[idx].setData(plot.x, plot.y[idx])
                            continue
                        plot.replot()
                        continue
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
                        plot.curve[idx].attach(plot)
                        plot.curve[idx].setPen(Qt.QPen(Qt.QColor(plot.color[idx % 16])))
                        plot.curve[idx].setStyle(Qwt.QwtPlotCurve.Dots)
                        pass

                    secs = integration_slice * self.integration_time
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
                break

            continue

        pass

    pass


class progressDialog(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.ui = Ui_Dialog1()
        self.ui.setupUi(self)

    def abort(self):
        if self.proc:
            self.status = 'ABORT'
            self.proc.terminate()
        else:
            self.reject()
            pass
        pass

    def update_status(self):
        conn = db.connect(host="ccs", port=3307,
                          db="correlator_control",
                          read_default_file="~/.my.cnf")

        cursor = conn.cursor()
        cursor.execute("UPDATE data_logbook" \
                           + " SET correlator_status='%s'" % self.status \
                           + " WHERE subjob_id=%d" % self.subjob)
        cursor.close()
        conn.commit()
        conn.close()
        pass

    def run(self, vex_file, ctrl_file, machine_file, rank_file):
        self.vex = Vex(vex_file)

        basename = os.path.splitext(ctrl_file)[0]
        log_file = basename + ".log"
        self.log_fp = open(log_file, 'w', 1)

        fp = open(ctrl_file, 'r')
        self.json_input = json.load(fp)
        fp.close()

        self.start = vex2time(self.json_input['start'])
        self.stop = vex2time(self.json_input['stop'])
        self.subjob = self.json_input['subjob']
        self.ui.progressBar.setRange(self.start, self.stop)
        self.secs = self.start
        self.scan = None
        self.plot = None
        self.status = 'CRASH'

        # Parse the rankfile to calculate the number of MPI processes
        # to start.  We simply count the number of lines that start
        # with "rank".
        ranks = 0
        fp = open(rank_file, 'r')
        r1 = re.compile(r'rank')
        for line in fp:
            m = r1.match(line)
            if m:
                ranks += 1
                pass
            continue
        fp.close()

        sfxc = '/home/sfxc/bin/sfxc'
        #sfxc = '/home/kettenis/opt/sfxc-ipp.mpich/bin/sfxc'
        args = ['mpirun', '--mca', 'btl_tcp_if_include', 'bond0,ib0,eth0',
                '--machinefile', machine_file, '--rankfile', rank_file,
                '--np', str(ranks), sfxc, ctrl_file, vex_file]
        #args = ['mpirun.mpich',
        #        '-machinefile', machine_file,
        #        '-np', str(ranks), sfxc, ctrl_file, vex_file]

        self.proc = subprocess.Popen(args, stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
        if not self.proc:
            return

        self.t = QtCore.QTimer()
        QtCore.QObject.connect(self.t, QtCore.SIGNAL("timeout()"), self.timeout)
        self.t.start(500)
        pass

    def timeout(self):
        output = self.proc.asyncread()
        if output:
            r1 = re.compile(r'(\d+y\d+d\d+h\d+m\d+s)')
            r2 = re.compile(r'Terminating nodes')
            for line in output.splitlines():
                m = r1.search(line)
                if m:
                    if not self.plot:
                        self.plot = WeightPlotWindow(self.vex, self.json_input)
                        self.plot.show()
                        pass
                    secs = vex2time(m.group())
                    if secs > self.secs:
                        self.secs = secs
                        tupletime = time.gmtime(self.secs)
                        strtime = time.strftime("%H:%M:%S", tupletime)
                        self.ui.timeEdit.setText(strtime)
                        self.ui.progressBar.setValue(self.secs)
                        for scan in self.vex['SCHED']:
                            if secs < vex2time(self.vex['SCHED'][scan]['start']):
                                break
                            current_scan = scan
                            continue
                        if current_scan != self.scan:
                            self.scan = current_scan
                            self.ui.scanEdit.setText(self.scan)
                            pass
                        pass
                    pass
                m = r2.search(line)
                if m:
                    self.ui.progressBar.setValue(self.stop)
                    pass
                continue
            self.ui.logEdit.append(output.rstrip())
            self.log_fp.write(output)
            pass

        self.proc.poll()
        if self.proc.returncode != None:
            self.t.stop()
            self.log_fp.close()
            if self.proc.returncode == 0:
                self.status = 'DONE'
                self.update_status()
                del self.plot
                self.accept()
                pass
            else:
                QtGui.QMessageBox.warning(self, "Aborted",
                                          "Correlation job finished unsucessfully.")
                self.update_status()
                del self.plot
                self.reject()
                sys.exit(1)
                pass
            pass

        pass

    pass

usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)

(options, args) = parser.parse_args()
if len(args) != 4:
    parser.error("incorrect number of arguments")
    pass

os.environ['TZ'] = 'UTC'
time.tzset()

app = QtGui.QApplication(sys.argv)
d = progressDialog()
d.run(args[0], args[1], args[2], args[3])
d.show()
sys.exit(app.exec_())
