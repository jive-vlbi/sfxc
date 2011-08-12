# Standard Python modules
import json
import os
import re
import optparse
import socket
import subprocess
import sys
import time
import urlparse

# MySQL
import MySQLdb as db

# JSON-RPC
import jsonrpclib

# JIVE Python modules
from vex_parser import Vex

class DataFlow:
    def __init__(self, vex, control_host=None, input_host=None):
        self.stations = []
        for station in vex['STATION']:
            self.stations.append(station)
            continue

        self.mtu = {}
        self.ipd = {}
        self.control_host = {}
        self.connect_host = {}
        self.input_host = {}

        conn = db.connect(host="ccs", db="ccs", user="jops")
        cursor = conn.cursor()
        for station in self.stations:
            cursor.execute("SELECT mtu FROM eVLBI_Params" \
                               + " WHERE e_station_name='%s'" % station)
            result = cursor.fetchone()
            if result:
                self.mtu[station] = result[0]
                pass

            cursor.execute("SELECT ipd FROM eVLBI_Params" \
                               + " WHERE e_station_name='%s'" % station)
            result = cursor.fetchone()
            if result:
                self.ipd[station] = result[0]
                pass

            cursor.execute("SELECT e_station_control_ip FROM eVLBI_Params" \
                               + " WHERE e_station_name='%s'" % station)
            result = cursor.fetchone()
            if result:
                self.control_host[station] = result[0]
                pass

            cursor.execute("SELECT assoc_ext_du_ip,sfxc_ip FROM SU_Params" \
                               + " WHERE assoc_e_station_name='%s'" % station)
            result = cursor.fetchone()
            if result:
                self.connect_host[station] = result[0]
                self.input_host[station] = result[1]
                pass

            continue

        cursor.execute("SELECT assoc_ext_du_ip,sfxc_ip,assoc_e_station_name"
                       + " FROM SU_Params")
        result = cursor.fetchone()
        for station in self.stations:
            if not station in self.connect_host:
                while result and result[2] in self.stations:
                    result = cursor.fetchone()
                    continue
                if result:
                    self.connect_host[station] = result[0]
                    self.input_host[station] = result[1]
                    result = cursor.fetchone()
                    pass
                pass
            continue

        cursor.close()
        conn.close()

        fanout = {}
        sample_rate = {}
        self.mk5_mode = {}
        for station in self.stations:
            das = vex['STATION'][station]['DAS']
            type = vex['DAS'][das]['record_transport_type']
            for scan in vex['SCHED']:
                mode = vex['SCHED'][scan]['mode']

                # Determine the sample rate.
                for freq in vex['MODE'][mode].getall('FREQ'):
                    if station in freq[1:]:
                        rate = vex['FREQ'][freq[0]]['sample_rate'].split()
                        sample_rate[station] = float(rate[0])
                        if rate[1] == 'Ks/sec':
                            sample_rate[station] *= 1e3
                        elif rate[1] == 'Ms/sec':
                            sample_rate[station] *= 1e6
                            pass
                        break
                    continue

                # Only look at the $TRACKS section for Mark5A.
                if type == 'Mark5A':
                    for tracks in vex['MODE'][mode].getall('TRACKS'):
                        if station in tracks[1:]:
                            tracks = vex['TRACKS'][tracks[0]]
                            if not tracks['track_frame_format'] == 'Mark4':
                                break
                            # Count the number of tracks.
                            # Do we need to support fanin as well?
                            sum = 0
                            for fanout_def in tracks.getall('fanout_def'):
                                assert(len(fanout_def) >= 5)
                                sum += (len(fanout_def) - 4)
                                fanout[station] = (len(fanout_def) - 4)
                                continue
                            self.mk5_mode[station] = ("mark4", "%d" % sum)
                            break
                        continue
                    pass

                # If we have a $BITSTREAMS section, assume we're dealing
                # with a Mark5B regardless what the VEX file says.
                for bitstreams in vex['MODE'][mode].getall('BITSTREAMS'):
                    if station in bitstreams[1:]:
                        bitstreams = vex['BITSTREAMS'][bitstreams[0]]
                        mask = 0
                        for stream_def in bitstreams.getall('stream_def'):
                            mask |= (1 << int(stream_def[2]))
                            continue
                        decimation = int(32e6 / sample_rate[station])
                        self.mk5_mode[station] = \
                            ("ext", "0x%08x" % mask, "%d" % decimation)
                        break
                    continue

                continue
            continue

        if control_host:
            self.direct = True
            self.control_host = control_host
            self.connect_host = input_host
            self.input_host = input_host
        else:
            self.direct = False
            pass

        self.commands = {}
        for station in vex['STATION']:
            # Initialization
            self.commands[station] = []
            command = "mode=%s" % self.mk5_mode[station][0]
            for arg in self.mk5_mode[station][1:]:
                command += ":%s" % arg
                continue
            command += ";"
            self.commands[station].append(command)
            if not self.mk5_mode[station][0] == "ext":
                data_rate = sample_rate[station] / (fanout[station] * 1e6)
                command = "play_rate=data:%f;" % data_rate
                self.commands[station].append(command)
                pass
            command = "net_protocol=udp;"
            self.commands[station].append(command)
            if station in self.mtu:
                command = "mtu=%d;" % self.mtu[station]
                self.commands[station].append(command)
                pass
            if station in self.ipd:
                command = "ipd=%d;" % self.ipd[station]
                self.commands[station].append(command)
                pass
            if station in self.connect_host:
                command = "in2net=connect:%s;" % self.connect_host[station]
                self.commands[station].append(command)
                pass

            continue

        return

    def send_commands_direct(self, station, commands):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.control_host[station], 2620))
        for command in commands:
            print command
            s.send(command)
            data = s.recv(1024)
            print repr(data)
            continue
        s.close()
        return

    def send_commands_proxy(self, station, commands):
        server = jsonrpclib.Server('http://ccsops:8080')
        results = server.send_commands(station, commands)
        for (command, result) in zip(commands, results):
            print command
            print result
            continue
        return

    def send_commands(self, station, commands):
        if self.direct:
            self.send_commands_direct(station, commands)
        else:
            self.send_commands_proxy(station, commands)
            pass
        return

    def setup(self, stations):
        for station in stations:
            self.send_commands(station, self.commands[station])
            continue
        return

    def start(self, stations):
        for station in stations:
            self.send_commands(station, ["in2net=on;"])
            continue
        return

    def stop(self, stations):
        for station in stations:
            self.send_commands(station, ["in2net=off;"])
            continue
        return

    def finalize(self, stations):
        for station in stations:
            self.send_commands(station, ["in2net=disconnect;"])
            continue
        return

    pass
