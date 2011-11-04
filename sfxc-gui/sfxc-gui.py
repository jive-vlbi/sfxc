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
from weightplot import WeightPlotWindow
from fringeplot import FringePlotWindow

# JIVE Python modules
from vex_parser import Vex
from cordata import CorrelatedData

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

class progressDialog(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.ui = Ui_Dialog1()
        self.ui.setupUi(self)

    def abort(self):
        if self.proc:
            self.status = 'ABORT'
            try:
                self.proc.terminate()
            except:
                self.update_status()
                self.reject()
                sys.exit(1)
                pass
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

    def run(self, vex_file, ctrl_file, machine_file, rank_file, reference):
        self.vex = Vex(vex_file)
        self.ctrl_file = ctrl_file
        self.reference = reference

        exper = self.vex['GLOBAL']['EXPER']

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
        self.time = self.start
        self.cordata = None
        self.scan = None
        self.wplot = None
        self.fplot = None
        self.status = 'CRASH'

        # Generate delays for this subjob.
        procs = {}
        success = True
        delay_directory = self.json_input['delay_directory']
        for station in self.json_input['stations']:
            path = urlparse.urlparse(delay_directory).path
            delay_file = path + '/' +  exper + '_' + station + '.del'
            args = ['generate_delay_model', '-a', vex_file, station,
                    delay_file, time2vex(self.start), time2vex(self.stop)]
            procs[station] = subprocess.Popen(args)
            continue
        for station in procs:
            procs[station].wait()
            if procs[station].returncode != 0:
                msg = "Delay model couldn't be generated for " + station + "."
                QtGui.QMessageBox.warning(self, "Aborted", msg)
                path = urlparse.urlparse(self.json_input['delay_directory']).path
                delay_file = path + '/' +  exper + '_' + station + '.del'
                os.remove(delay_file)
                success = False
                pass
            continue
        if not success:
            sys.exit(1)
            pass

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
        args = ['mpirun',
                '--mca', 'btl_tcp_if_include', 'bond0,ib0,eth0,eth2.4',
                '--machinefile', machine_file, '--rankfile', rank_file,
                '--np', str(ranks), sfxc, ctrl_file, vex_file]
        if os.environ['LOGNAME'] == 'kettenis':
            sfxc = '/home/kettenis/opt/sfxc.mpich/bin/sfxc'
            args = ['mpirun.mpich',
                    '-machinefile', machine_file,
                    '-np', str(ranks), sfxc, ctrl_file, vex_file]
            pass

        self.proc = subprocess.Popen(args, stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
        if not self.proc:
            return

        self.t = QtCore.QTimer()
        QtCore.QObject.connect(self.t, QtCore.SIGNAL("timeout()"), self.timeout)
        self.t.start(500)
        pass

    def timeout(self):
        if self.cordata:
            self.cordata.read()
            if self.time < self.cordata.current_time:
                self.time = self.cordata.current_time
                tupletime = time.gmtime(self.time)
                strtime = time.strftime("%H:%M:%S", tupletime)
                self.ui.timeEdit.setText(strtime)
                self.ui.progressBar.setValue(self.time)
                for scan in self.vex['SCHED']:
                    if self.time < vex2time(self.vex['SCHED'][scan]['start']):
                        break
                    current_scan = scan
                    continue
                if current_scan != self.scan:
                    self.scan = current_scan
                    self.ui.scanEdit.setText(self.scan)
                    pass
                pass
            pass
        output = self.proc.asyncread()
        if output:
            r1 = re.compile(r'(\d+y\d+d\d+h\d+m\d+s)')
            r2 = re.compile(r'Terminating nodes')
            for line in output.splitlines():
                m = r1.search(line)
                if m:
                    if not self.cordata:
                        output_file = urlparse.urlparse(self.json_input['output_file']).path
                        try:
                            if self.json_input['pulsar_binning']:
                                output_file = output_file + '.bin0'
                                pass
                        except:
                            pass
                        self.cordata = CorrelatedData(self.vex, output_file)
                        pass
                    if not self.wplot:
                        self.wplot = WeightPlotWindow(self.vex, [self.ctrl_file], True)
                        self.wplot.show()
                        pass

                    if not self.fplot:
                        self.fplot = FringePlotWindow(self.vex, [self.ctrl_file], self.reference)
                        self.fplot.show()
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
                self.accept()
                sys.exit(0)
                pass
            else:
                self.update_status()
                self.reject()
                if self.status == 'ABORT':
                    sys.exit(1)
                else:
                    sys.exit(0)
                    pass
                pass
            pass

        return

    pass

usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-r", "--reference", dest="reference",
                  default="", type="string",
                  help="Reference station",
                  metavar="STATION")

(options, args) = parser.parse_args()
if len(args) != 4:
    parser.error("incorrect number of arguments")
    pass

os.environ['TZ'] = 'UTC'
time.tzset()

app = QtGui.QApplication(sys.argv)
d = progressDialog()
d.run(args[0], args[1], args[2], args[3], options.reference)
d.show()
sys.exit(app.exec_())
