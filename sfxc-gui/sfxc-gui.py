#! /usr/bin/python

# Standard Python modules
from datetime import datetime, timedelta
import json
import os
import optparse
import re
import socket
import asyncsubprocess as subprocess
import struct
import sys
import signal
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
from vex import Vex
from cordata import CorrelatedData

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

class progressDialog(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent, QtCore.Qt.WindowStaysOnTopHint)
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
        if self.subjob == -1:
            return

        conn = db.connect(host="db0", port=3306,
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

    def update_output(self):
        if self.subjob == -1:
            return

        try:
            if self.json_input['pulsar_binning']:
                return
        except:
            pass
        try:
            if self.json_input['multi_phase_center']:
                return
        except:
            pass

        conn = db.connect(host="db0", port=3306,
                          db="correlator_control",
                          read_default_file="~/.my.cnf")

        cursor = conn.cursor()
        output_file = self.json_input['output_file']
        cursor.execute("INSERT INTO data_output (subjob_id, output_uri)" \
                           + " VALUES (%d, '%s')" % (self.subjob, output_file))
        cursor.close()
        conn.commit()
        conn.close()
        pass

    def get_job(self):
        if self.subjob == -1:
            return -1

        conn = db.connect(host="db0", port=3306,
                          db="correlator_control",
                          read_default_file="~/.my.cnf")

        cursor = conn.cursor()
        cursor.execute("SELECT job_id FROM cj_subjob " \
                           + " WHERE subjob_id=%d" % self.subjob)
        result = cursor.fetchone()
        if result:
            return result[0]
        return -1

    def start_reader(self, station):
        # Check if mk5read is already running and bail out if it is.
        args = ["ssh", self.input_host[station], "pgrep", "mk5read"]
        if subprocess.call(args) == 0:
            return None

        args = ["ssh", self.input_host[station], "bin/mk5read"]
        log = open(station + "-reader.log", 'w')
        p = subprocess.Popen(args, stdout=log, stderr=log)
        return p

    def start_readers(self):
        self.readers = {}
        for station in self.json_input['stations']:
            if self.json_input["data_sources"][station][0].startswith("mk5"):
                reader = self.start_reader(station)
                if reader:
                    self.readers[station] = reader
                    pass
                pass
            continue
        return

    def stop_readers(self):
        # Stop the readers, but only if we started them
        for station in self.readers:
            args = ["ssh", self.input_host[station], "pkill", "mk5read"]
            subprocess.call(args)
            continue
        for station in self.readers:
            self.readers[station].wait()
            continue
        return

    def run(self, vex_file, ctrl_file, machine_file, rank_file, options):
        self.vex = Vex(vex_file)
        self.ctrl_file = ctrl_file
        self.evlbi = options.evlbi
        self.reference = options.reference
        self.timeout_interval = options.timeout_interval

        # Disable the watchdog for e-VLBI.  We don't want to stop the
        # correlator just because there is a longish gap.
        if self.evlbi:
            self.timeout_interval = 0
            pass

        exper = self.vex['GLOBAL']['EXPER']
        exper = self.vex['EXPER'][exper]['exper_name']

        basename = os.path.splitext(ctrl_file)[0]
        log_file = basename + ".log"
        self.log_fp = open(log_file, 'w', 1)

        fp = open(ctrl_file, 'r')
        self.json_input = json.load(fp)
        fp.close()

        # When doing e-VLBI we don't need to generate delays for the past.
        if self.evlbi:
            self.json_input['start'] = time2vex(time.time())
            pass

        self.start = vex2time(self.json_input['start'])
        self.stop = vex2time(self.json_input['stop'])
        self.subjob = self.json_input.get('subjob', -1)
        self.ui.progressBar.setRange(self.start, self.stop)
        self.time = self.start
        self.cordata = None
        self.scan = None
        self.wplot = None
        self.fplot = None
        self.status = 'CRASH'

        # Parse the rankfile to figure out wher the input node for
        # each station runs.
        self.stations = self.json_input['stations']
        self.input_host = {}
        fp = open(rank_file, 'r')
        r1 = re.compile(r'rank (\d*)=(.*) slot=')
        for line in fp:
            m = r1.match(line)
            if not m:
                continue
            rank = int(m.group(1))
            if rank > 2 and rank < 3 + len(self.stations):
                self.input_host[self.stations[rank - 3]] = m.group(2)
                continue
            continue
        fp.close()

        if not self.evlbi:
            self.start_readers()
            pass

        # Generate delays for this subjob.
        if not options.skip_generate_delays:
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
            pass

        # When doing e-VLBI we want to start correlating a few seconds
        # from "now", but we have to make sure we're not in a gap
        # between scans.
        if self.evlbi:
            start = time.time() + 15
            for scan in self.vex['SCHED']:
                # Loop over all the "station" parameters in the scan,
                # figuring out the real length of the scan.
                start_time = stop_time = 0
                for transfer in self.vex['SCHED'][scan].getall('station'):
                    station = transfer[0]
                    stop_time = max(stop_time, int(transfer[2].split()[0]))
                    continue

                # Figure out the real start and stop time.
                start_time += vex2time(self.vex['SCHED'][scan]['start'])
                stop_time += vex2time(self.vex['SCHED'][scan]['start'])

                start = max(start, start_time)
                if start < stop_time:
                    break
                continue

            # Write out the control file with a modified start time.
            self.json_input['start'] = time2vex(start)
            os.unlink(ctrl_file)
            fp = open(ctrl_file, 'w')
            json.dump(self.json_input, fp, indent=4)
            fp.close()
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

        if self.subjob != -1:
            self.ui.jobEdit.setText(str(self.get_job()))
            self.ui.subjobEdit.setText(str(self.subjob))
            pass

        sfxc = '/home/sfxc/bin/sfxc'
        args = ['mpirun',
                '--mca', 'btl_tcp_if_include', 'bond0,eth0,eth2.4',
                '--mca', 'oob_tcp_if_exclude', 'eth1,eth2,eth3',
                '--machinefile', machine_file, '--rankfile', rank_file,
                '--np', str(ranks), sfxc, ctrl_file, vex_file]
        if os.environ['LOGNAME'] == 'kettenis':
            sfxc = '/home/kettenis/opt/sfxc.openmpi/bin/sfxc'
            args = ['/home/sfxc/bin/mpirun',
                    '--mca', 'btl_tcp_if_include', 'bond0,eth0,eth2.4',
                    '--mca', 'oob_tcp_if_exclude', 'eth1,eth2,eth3',
                    '--machinefile', machine_file, '--rankfile', rank_file,
                    '--np', str(ranks), sfxc, ctrl_file, vex_file]
            pass
        elif os.environ['LOGNAME'] == 'keimpema':
            sfxc = '/home/keimpema/sfxc/bin/sfxc'
            args = ['mpirun',
                    '--mca', 'btl_tcp_if_include', 'bond0,eth0,eth2.4',
                    '--mca', 'oob_tcp_if_exclude', 'eth1,eth2,eth3',
                    '--machinefile', machine_file, '--rankfile', rank_file,
                    '--np', str(ranks), sfxc, ctrl_file, vex_file]
        self.proc = subprocess.Popen(args, stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
        if not self.proc:
            return

        # Update the map-on-the-wall.
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(1)
            s.connect(("wwpad", 4004))
            command = ",".join(self.json_input['stations']) + "\n"
            s.send(command)
            s.recv(1024)
            s.close()
        except:
            pass

        self.monitor_time = time.time()
        self.monitor_pos = 0
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
            r1 = re.compile(r'Starting correlation')
            r2 = re.compile(r'Terminating nodes')
            r3 = re.compile(r'^Node #(\d+) fatal error.*Could not find header')

            for line in output.splitlines():
                m = r1.search(line)
                if m:
                    if not self.cordata:
                        output_file = urlparse.urlparse(self.json_input['output_file']).path
                        try:
                            if self.json_input['multi_phase_center']:
                                source = None
                                for scan in self.vex['SCHED']:
                                    if self.start >= vex2time(self.vex['SCHED'][scan]['start']):
                                        source = self.vex['SCHED'][scan]['source']
                                        pass
                                    continue
                                if source:
                                    output_file = output_file + '_' + source
                                    pass
                                pass
                        except:
                            pass
                        try:
                            if self.json_input['pulsar_binning']:
                                output_file = output_file + '.bin1'
                                pass
                        except:
                            pass
                        self.cordata = CorrelatedData(self.vex, output_file, True)
                        if not self.wplot:
                            self.wplot = WeightPlotWindow(self.vex, [self.ctrl_file], self.cordata, True)
                            self.wplot.show()
                            pass
                        if not self.fplot and self.cordata:
                            self.fplot = FringePlotWindow(self.vex, [self.ctrl_file], self.cordata, self.reference, self.evlbi)
                            self.fplot.show()
                            pass
                        self.update_output()
                        pass
                    pass
                m = r2.search(line)
                if m:
                    self.ui.progressBar.setValue(self.stop)
                    pass
                m = r3.match(line)
                if m:
                    station = self.stations[int(m.group(1)) - 3]
                    unit = None
                    r = re.compile(r'10\.88\.1\.(\d+)')
                    m = r.match(self.input_host[station])
                    if m:
                        unit = int(m.group(1)) - 200
                        pass
                    if unit != None:
                        print >>sys.stderr, "Error: Disk problem with %s in unit %d" % (station, unit)
                    else:
                        print >>sys.stderr, "Error: Disk Problem with %s" % (station)
                        pass
                    pass
                continue
            self.ui.logEdit.append(output.rstrip())
            self.log_fp.write(output)
            pass

        # Check if SFXC process is frozen
        curtime = time.time()
        if (self.timeout_interval > 0) and (curtime - self.monitor_time > self.timeout_interval):
            self.monitor_time = curtime
            terminate = False
            if (self.cordata == None) or (self.cordata.fp == None):
                terminate = True
            else:
                newpos = self.cordata.fp.tell()
                if newpos == self.monitor_pos: 
                    terminate = True
                self.monitor_pos = newpos
            if terminate:
                os.kill(self.proc.pid, signal.SIGTERM)
                i = 0
                while (i < 15) and (self.proc.poll() == None):
                    time.sleep(1)
                    i += 1
                if self.proc.poll() == None:
                    os.kill(self.proc.pid, signal.SIGKILL)
        self.proc.poll()
        if self.proc.returncode != None:
            self.t.stop()
            self.log_fp.close()

            if not self.evlbi:
                self.stop_readers()
                pass

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

usage = "usage: %prog [options] vexfile ctrlfile machinefile rankfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-r", "--reference", dest="reference",
                  default="", type="string",
                  help="Reference station",
                  metavar="STATION")
parser.add_option("-e", "--evlbi", dest="evlbi",
                  action="store_true", default=False,
                  help="e-VLBI")
parser.add_option("-s", "--skip-generate-delays", dest="skip_generate_delays",
                  action="store_true", default=False,
                  help="Do not generate delay files")
parser.add_option("-i", "--timeout-interval", dest="timeout_interval",
                  type='int', default='300',
                  help="After how many seconds of inactivity the job is terminated, setting to zero disables. Default = 300")

(options, args) = parser.parse_args()
if len(args) != 4:
    parser.error("incorrect number of arguments")
    pass

os.environ['TZ'] = 'UTC'
time.tzset()

app = QtGui.QApplication(sys.argv)
d = progressDialog()
d.run(args[0], args[1], args[2], args[3], options)
d.show()
sys.exit(app.exec_())
