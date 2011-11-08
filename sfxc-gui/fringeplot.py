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
    def updateLegend(self, legend):
        Qwt.QwtPlotCurve.updateLegend(self, legend)
        item = legend.find(self)
        if item:
            pen = Qt.QPen(self.pen())
            pen.setWidth(3)
            item.setCurvePen(pen)
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
        scaleDraw = BottomScaleDraw(number_channels)
        self.setAxisScaleDraw(Qwt.QwtPlot.xBottom, scaleDraw)
        scaleDiv = Qwt.QwtScaleDiv(0, number_channels, [],
                                   [number_channels / 4, 3 * number_channels / 4],
                                   [0, number_channels / 2, number_channels])
        self.setAxisScaleDiv(Qwt.QwtPlot.xBottom, scaleDiv)
        self.setAxisTitle(Qwt.QwtPlot.yLeft, station1 + '-' + station2)
        self.setAxisMaxMajor(Qwt.QwtPlot.yLeft, 2)
        self.curve = {}

        self.xxxcurve = Qwt.QwtPlotCurve("XXX")
        self.xxxcurve.attach(self)

        self.grid = Qwt.QwtPlotGrid()
        self.grid.setPen(Qt.QPen(Qt.Qt.lightGray))
        self.grid.enableY(False)
        self.grid.setXDiv(scaleDiv)
        self.grid.attach(self)

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
    def __init__(self, vex, ctrl_files, reference=None, *args):
        Qt.QWidget.__init__(self, *args)

        exper = vex['GLOBAL']['EXPER']
        exper = vex['EXPER'][exper]['exper_name']
        self.setWindowTitle(exper + " Fringes")

        self.integration_slice = 0
        self.reference = reference

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
                if json_input['pulsar_binning']:
                    output_file = output_file + '.bin0'
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
            if history == 32:
                act.setChecked(True)
            grp.addAction(act)
            menu.addAction(act)
            continue

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
            continue
        legend = FringePlotLegend()
        legend.setItemMode(Qwt.QwtLegend.CheckableItem)
        self.plots[-1].insertLegend(legend, Qwt.QwtPlot.ExternalLegend)

        self.box = Qt.QVBoxLayout(self)
        self.box.setMenuBar(menubar)
        self.box.addLayout(self.layout)
        self.box.addWidget(self.plots[-1].legend())

        self.cordata = CorrelatedData(vex, self.output_files[self.output_file])
        self.output_file += 1

        self.startTimer(500)
        self.resize(600, len(stations) * 100 + 50)
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

    def stretch(self):
        self.plots[-1].enableAxis(Qwt.QwtPlot.xBottom, True)
        self.plots[-1].xxxcurve.setItemAttribute(Qwt.QwtPlotItem.Legend, False)
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
                pol1 = (idx >> 0) & 1
                pol2 = (idx >> 1) & 1
                usb = (idx >> 2) & 1
                band = (idx >> 3) & 0x1f
                if not pol1 == pol2:
                    continue

                if not idx in plot.curve:
                    title = "SB%d" % ((idx >> 1) / 2)
                    if (idx >> 1) % 2 == 0:
                        title += " RR"
                    else:
                        title += " LL"
                        pass

                    pen = Qt.QPen()
                    pen.setColor(Qt.QColor(plot.color[(idx >> 1) % 16]))
                    pen.setWidth(1)

                    plot.curve[idx] = FringePlotCurve(title)
                    plot.curve[idx].setData(range(self.cordata.number_channels),
                                            range(self.cordata.number_channels))
                    plot.curve[idx].setPen(pen)
                    plot.curve[idx].attach(plot)
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
                b = np.fft.fft(a, self.cordata.number_channels)
                c = b[(self.cordata.number_channels / 2):]
                d = b[0:(self.cordata.number_channels / 2)]
                e = np.concatenate((c, d))
                f = np.absolute(e)
                g = f / np.sum(f)
                if station == baseline[0]:
                    g = g[::-1]
                    pass

                plot.curve[idx].setData(range(self.cordata.number_channels), g)
                continue
            plot.replot()
            continue
        return

    def timerEvent(self, e):
        self.cordata.read()
        if self.cordata.integration_slice > self.integration_slice:
            self.integration_slice = self.cordata.integration_slice
            self.replot()
        return

    pass


if __name__ == '__main__':
    usage = "usage: %prog [options] vexfile ctrlfile"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-r", "--reference", dest="reference",
                      default="", type="string",
                      help="Reference station",
                      metavar="STATION")

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

    plot = FringePlotWindow(vex, ctrl_files, options.reference)
    plot.show()

    sys.exit(app.exec_())
