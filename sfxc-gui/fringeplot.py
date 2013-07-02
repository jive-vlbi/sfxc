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
from cordata import CorrelatedData

# NumPy
import numpy as np

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

class BottomScaleDraw(Qwt.QwtScaleDraw):
    def __init__(self, number_channels, *args):
        self.number_channels = number_channels
        Qwt.QwtScaleDraw.__init__(self, *args)
        self.setLabelAlignment(Qt.Qt.AlignLeft | Qt.Qt.AlignBottom)
        return

    def drawLabel(self, p, val):
        align = self.labelAlignment()
        if val == self.number_channels:
            self.setLabelAlignment(Qt.Qt.AlignLeft | Qt.Qt.AlignBottom)
        else:
            self.setLabelAlignment(Qt.Qt.AlignHCenter | Qt.Qt.AlignBottom)
            pass
        Qwt.QwtScaleDraw.drawLabel(self, p, val)
        self.setLabelAlignment(align)
        pass
    pass

class LeftScaleDraw(Qwt.QwtScaleDraw):
    def __init__(self, *args):
        Qwt.QwtScaleDraw.__init__(self, *args)
        return

    def label(self, val):
        if val == 0:
            return Qwt.QwtText("    0.0")
        return Qwt.QwtScaleDraw.label(self, val)

    pass

class FringePlotCurve(Qwt.QwtPlotCurve):
    def __init__(self, tip, *args):
        Qwt.QwtPlotCurve.__init__(self, *args)
        self.tip = tip
        return

    def updateLegend(self, legend):
        Qwt.QwtPlotCurve.updateLegend(self, legend)
        item = legend.find(self)
        if item:
            pen = Qt.QPen(self.pen())
            pen.setWidth(3)
            item.setCurvePen(pen)
            if self.tip:
                item.setToolTip(self.tip)
                pass
            pass
        return

    pass

class FringePlotLegend(Qwt.QwtLegend):
    def sizeHint(self):
        size = Qwt.QwtLegend.sizeHint(self)
        numrows = min(self.contentsWidget().layout().numRows(), 4)
        if numrows > 0:
            return Qt.QSize(size.width(), numrows * size.height())
        return size;

    pass

class FringePlotPicker(Qwt.QwtPlotPicker):
    def trackerText(self, point):
        map = self.plot().canvasMap(Qwt.QwtPlot.xBottom)
        lags = map.invTransform(point.x()) - self.number_channels
        delay = lags / self.sample_rate
        s = "%.1f" % (delay * 1e6)
        return Qwt.QwtText(Qt.QString(s) + " " + Qt.QChar(0xb5))

    pass

class FringePlot(Qwt.QwtPlot):
    color = [ "#bae4b3", "#74c476", "#31a354", "#006d2c",
              "#bdd7e7", "#6baed6", "#3182bd", "#08519c",
              "#fcae91", "#fb6a4a", "#de2d26", "#a50f15",
              "#cccccc", "#999999", "#666666", "#000000" ]

    def __init__(self, parent, station1, station2, number_channels, *args):
        Qwt.QwtPlot.__init__(self, *args)

        self.setCanvasBackground(Qt.Qt.white)

        self.x = []
        self.y = {}
        self.station = station2

        scaleDraw = LeftScaleDraw()
        self.setAxisScaleDraw(Qwt.QwtPlot.yLeft, scaleDraw)
        scaleDraw = BottomScaleDraw(2 * number_channels)
        self.setAxisScaleDraw(Qwt.QwtPlot.xBottom, scaleDraw)
        scaleDiv = Qwt.QwtScaleDiv(0, 2 * number_channels, [],
                                   [number_channels / 2, 3 * number_channels / 2],
                                   [0, number_channels, 2 * number_channels])
        self.setAxisScaleDiv(Qwt.QwtPlot.xBottom, scaleDiv)
        self.setAxisTitle(Qwt.QwtPlot.yLeft, station1 + '-' + station2)
        self.setAxisMaxMajor(Qwt.QwtPlot.yLeft, 2)
        self.curve = {}

        self.centercurve = Qwt.QwtPlotCurve("XXX")
        x = [ number_channels, number_channels]
        y = [ -1, 2 ]
        self.centercurve.setData(x, y)
        self.centercurve.setPen(Qt.Qt.lightGray)
        self.centercurve.setItemAttribute(Qwt.QwtPlotItem.AutoScale, False)
        self.centercurve.attach(self)

        self.connect(self, Qt.SIGNAL("legendChecked(QwtPlotItem*,bool)"),
                     self.toggleCurve)
        self.parent = parent
        return

    def toggleCurve(self, curve, state):
        for idx in self.curve:
            if self.curve[idx] == curve:
                break
            continue
            
        for plot in self.parent.plots:
            try:
                plot.curve[idx].setVisible(not state)
                plot.replot()
            except:
                pass
            continue
        return

    pass

class FringePlotWindow(Qt.QWidget):
    def __init__(self, vex, ctrl_files, cordata, reference=None, evlbi=False, integrations=32, *args):
        Qt.QWidget.__init__(self, *args)

        exper = vex['GLOBAL']['EXPER']
        exper = vex['EXPER'][exper]['exper_name']
        self.setWindowTitle(exper + " Fringes")

        self.integration_slice = 0
        self.reference = reference
        self.evlbi = evlbi
        self.integrations = integrations

        self.output_file = 0
        self.output_files = []
        stations = []
        for ctrl_file in ctrl_files:
            fp = open(ctrl_file, 'r')
            json_input = json.load(fp)
            fp.close()
            for station in json_input['stations']:
                if station not in stations:
                    stations.append(station)
                    pass
                continue
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
            continue

        fp = open(ctrl_files[0], 'r')
        json_input = json.load(fp)
        fp.close()
        number_channels = json_input['number_channels']

        if not self.reference:
            self.reference = stations[0]
            pass

        if not self.integrations in [1, 2, 4, 8, 16, 32]:
            self.integrations = 32
            pass

        self.cross = False

        # Create a sorted list of frequencies
        self.frequencies = []
        self.sample_rate = 1e12
        station = self.reference
        for scan in vex['SCHED']:
            mode = vex['SCHED'][scan]['mode']
            for freq in vex['MODE'][mode].getall('FREQ'):
                if station in freq[1:]:
                    value = vex['FREQ'][freq[0]]['sample_rate'].split()
                    sample_rate = float(value[0])
                    if value[1] == 'Gs/sec':
                        sample_rate *= 1e9
                    elif value[1] == 'Ms/sec':
                        sample_rate *= 1e6
                    if sample_rate < self.sample_rate:
                        self.sample_rate = sample_rate
                        pass
                    channels = vex['FREQ'][freq[0]].getall('chan_def')
                    for chan_def in channels:
                        value = chan_def[1].split()
                        frequency = float(value[0])
                        if value[1] == 'GHz':
                            frequency *= 1e9
                        elif value[1] == 'MHz':
                            frequency *= 1e6
                        elif value[1] == 'KHz':
                            frequency *= 1e3
                            pass
                        if not frequency in self.frequencies:
                            self.frequencies.append(frequency)
                            pass
                        continue
                    break
                continue
            break
        self.frequencies.sort()

        menubar = Qt.QMenuBar(self)
        menu = menubar.addMenu("&Reference")
        self.connect(menu, Qt.SIGNAL("triggered(QAction *)"),
                     self.setReference)
        grp = Qt.QActionGroup(menu)
        for station in stations:
            act = Qt.QAction(station, menu)
            act.setCheckable(True)
            if act.text() == self.reference:
                act.setChecked(True)
            grp.addAction(act)
            menu.addAction(act)
            continue
        menu = menubar.addMenu("&Integrations")
        self.connect(menu, Qt.SIGNAL("triggered(QAction *)"),
                     self.setIntegrations)
        grp = Qt.QActionGroup(menu)
        for history in [1, 2, 4, 8, 16, 32]:
            act = Qt.QAction(str(history), menu)
            act.setCheckable(True)
            if history == self.integrations:
                act.setChecked(True)
            grp.addAction(act)
            menu.addAction(act)
            continue
        menu = menubar.addMenu("&Polarisations")
        self.connect(menu, Qt.SIGNAL("triggered(QAction *)"),
                     self.setPolarizations)
        grp = Qt.QActionGroup(menu)
        act = Qt.QAction("&Parallel Hands", menu)
        act.setCheckable(True)
        act.setChecked(True)
        grp.addAction(act)
        menu.addAction(act)
        if json_input["cross_polarize"]:
            act = Qt.QAction("&Cross Hands", menu)
            act.setCheckable(True)
            grp.addAction(act)
            menu.addAction(act)
            pass

        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue
        self.stations.sort()

        self.plots = []
        self.layout = Qt.QGridLayout()
        for station in stations:
            if station == self.reference:
                continue
            plot = FringePlot(self, self.reference, station, number_channels)
            plot.enableAxis(Qwt.QwtPlot.xBottom, False)
            self.layout.addWidget(plot)
            self.layout.setRowStretch(self.layout.rowCount() - 1, 100)
            self.plots.append(plot)
            picker = FringePlotPicker(plot.canvas())
            picker.setSelectionFlags(Qwt.QwtPicker.PointSelection | Qwt.QwtPicker.DragSelection)
            picker.setRubberBandPen(Qt.QColor(Qt.Qt.red))
            picker.setRubberBand(Qwt.QwtPicker.VLineRubberBand)
            picker.setMousePattern(Qwt.QwtPicker.MouseSelect1, Qt.Qt.LeftButton)
            picker.setTrackerMode(Qwt.QwtPicker.ActiveOnly)
            picker.sample_rate = self.sample_rate
            picker.number_channels = number_channels
            continue
        legend = FringePlotLegend()
        legend.setItemMode(Qwt.QwtLegend.CheckableItem)
        self.plots[-1].insertLegend(legend, Qwt.QwtPlot.ExternalLegend)

        self.box = Qt.QVBoxLayout(self)
        self.box.setMenuBar(menubar)
        self.box.addLayout(self.layout)
        self.box.addWidget(self.plots[-1].legend())
        self.menubar = menubar

        if cordata:
            self.cordata = cordata
        else:
            self.cordata = CorrelatedData(vex, self.output_files[self.output_file])
            pass
        self.cordata.history = self.integrations
        self.output_file += 1

        self.scp_proc = None

        self.startTimer(500)
        self.resize(620, len(stations) * 100 + 50)
        pass

    def setReference(self, act):
        reference = str(act.text())
        for plot in self.plots:
            if plot.station == reference:
                plot.station = self.reference
                break
            continue
        self.reference = reference
        for plot in self.plots:
            plot.setAxisTitle(Qwt.QwtPlot.yLeft,
                              self.reference + '-' + plot.station)
            continue
        self.replot()
        return

    def setIntegrations(self, act):
        self.cordata.history = int(str(act.text()))
        self.cordata.correlations = {}
        return

    def setPolarizations(self, act):
        if str(act.text()) == "&Parallel Hands":
            self.cross = False
        else:
            self.cross = True
            pass
        for plot in self.plots:
            for idx in plot.curve:
                plot.curve[idx].detach()
                continue
            plot.curve = {}
            continue
        self.replot()
        return

    def stretch(self):
        self.plots[-1].enableAxis(Qwt.QwtPlot.xBottom, True)
        self.plots[-1].centercurve.setItemAttribute(Qwt.QwtPlotItem.Legend, False)
        height = self.plots[-1].height()
        canvasHeight = self.plots[-1].plotLayout().canvasRect().height()
        fixedHeight = height - canvasHeight
        if fixedHeight > 0:
            height = self.layout.contentsRect().height()
            height -= (len(self.plots) - 1) * self.layout.verticalSpacing()
            height /= len(self.plots)
            if height > 0:
                stretch = (height + fixedHeight) * 110 / height
                self.layout.setRowStretch(self.layout.rowCount() - 1, stretch)
                pass
            pass
        self.plots[-1].legend().updateGeometry()
        return

    def resizeEvent(self, e):
        self.stretch()
        Qt.QWidget.resizeEvent(self, e)
        pass

    def replot(self):
        time = self.cordata.time
        correlations = self.cordata.correlations
        for baseline in correlations:
            if not self.reference in baseline:
                continue
            station = baseline[0]
            if station == self.reference:
                station = baseline[1]
                pass
            if station == self.reference:
                continue
            for plot in self.plots:
                if plot.station == station:
                    break
            for idx in correlations[baseline]:
                if station == baseline[0]:
                    pol1 = (idx >> 1) & 1
                    pol2 = (idx >> 0) & 1
                else:
                    pol1 = (idx >> 0) & 1
                    pol2 = (idx >> 1) & 1
                    pass
                usb = (idx >> 2) & 1
                band = (idx >> 3) & 0x1f
                if self.cross and pol1 == pol2:
                    continue
                if not self.cross and not pol1 == pol2:
                    continue

                plot_idx = (band << 3) | (usb << 2) | (pol2 << 1) | (pol1 << 0)
                if not plot_idx in plot.curve:
                    title = "SB%d" % ((plot_idx >> 1) / 2)
                    if pol1 == 0:
                        title += " R"
                    else:
                        title += " L"
                        pass
                    if pol2 == 0:
                        title += "R"
                    else:
                        title += "L"
                        pass

                    try:
                        tip = "%.2f MHz" % (self.frequencies[band] * 1e-6)
                        if usb:
                            tip += " USB"
                        else:
                            tip += " LSB"
                            pass
                    except:
                        tip = ""

                    pen = Qt.QPen()
                    pen.setColor(Qt.QColor(plot.color[(plot_idx >> 1) % 16]))
                    pen.setWidth(1)

                    plot.curve[plot_idx] = FringePlotCurve(tip, title)
                    plot.curve[plot_idx].setData(range(self.cordata.number_channels),
                                            range(self.cordata.number_channels))
                    plot.curve[plot_idx].setPen(pen)
                    plot.curve[plot_idx].attach(plot)
                    if plot == self.plots[-1]:
                        self.stretch()
                        pass

                    # Sort curves by detaching them all and
                    # reattach them in the right order.
                    for i in plot.curve:
                        plot.curve[i].detach()
                        plot.curve[i].attach(plot)
                        continue
                    pass

                a = correlations[baseline][idx].sum(axis=0)
                if station == baseline[1]:
                    a = np.conj(a)
                    pass
                b = np.fft.ifft(a, 2 * self.cordata.number_channels)
                c = b[self.cordata.number_channels:]
                d = b[0:self.cordata.number_channels]
                e = np.concatenate((c, d))
                f = np.absolute(e)

                plot.curve[plot_idx].setData(range(2 * self.cordata.number_channels), f)
                continue
            plot.replot()
            continue
        self.refresh = True
        return

    def timerEvent(self, e):
        self.cordata.read()
        if self.cordata.integration_slice > self.integration_slice:
            self.integration_slice = self.cordata.integration_slice
            self.replot()
            if self.evlbi:
                if not self.scp_proc:
                    pixmap = Qt.QPixmap().grabWidget(self, 0, self.menubar.height())
                    pixmap.save("fringe.png")
                    log = open("scp.log", 'a')
                    args = ["scp", "fringe.png", "services.jive.nl:/var/www/sfxc/"]
                    self.scp_proc = subprocess.Popen(args, stdout=log, stderr=log)
                else:
                    self.scp_proc.poll()
                    if self.scp_proc.returncode != None:
                        self.scp_proc = None
                        pass
                    pass
                pass
            pass
        return

    pass


if __name__ == '__main__':
    usage = "usage: %prog [options] vexfile ctrlfile"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-r", "--reference", dest="reference",
                      default="", type="string",
                      help="Reference station",
                      metavar="STATION")
    parser.add_option("-e", "--evlbi", dest="evlbi",
                      action="store_true", default=False,
                      help="e-VLBI")
    parser.add_option("-i", "--integrations", dest="integrations",
                      default=32, type="int",
                      help="Number of integrations",
                      metavar="N")

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

    plot = FringePlotWindow(vex, ctrl_files, None, options.reference, options.evlbi, options.integrations)
    plot.show()

    sys.exit(app.exec_())
