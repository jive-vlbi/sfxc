#!/usr/bin/python

"""
Chop up the job in time slices such that we end up with chunks of data
that are (roughly) a given size.
"""

from Vex import Vex as AntlrVex
import time
from time import strptime, strftime, mktime, localtime

# Conversion factors for sample rates
sample_rate_units = {'s/sec': 1, 'Ks/sec': 1e3, 'Ms/sec': 1e6, 'Gs/sec': 1e9}

# Relative overhead from headers
data_format_overhead = {'Mark4': 1.0, 'VLBA': 1.008}

class Hell(Exception): pass

class Vex:
    def __init__(self, name):
        self._vex = AntlrVex(name)
        return

    def __getitem__(self, key):
        return self._vex[key]

    def get_mode(self, scan):
        """Return the name of the mode for SCAN."""
        _sched = self._vex['SCHED']
        return _sched[scan]['mode']

    def get_freq(self, mode, station):
        """Return the name of the entry in the $FREQ block for MODE
        and STATION."""
        _mode = self._vex['MODE']
        for freq in _mode[mode].getall('FREQ'):
            if station in freq[1:]:
                return freq[0]
        else:
            raise Hell
#
    def get_data_start(self, scan, station):
        """Return the starting position (in GB) of the data recording in the 
$SCHED block for SCAN and STATION."""
        _data = self._vex['SCHED'][scan].getall('station')
        for items in _data:
            sublist = items
            if sublist[0] == station:
                return float(sublist[3].strip(' GB'))
        else:
            raise Hell
#
    def get_data_format(self, scan,station):
        """Return the data format for STATION."""
        mode = self._vex['SCHED'][scan]["mode"]
        track = self._vex['MODE'][mode].getall('TRACKS')
        for items in track:
            if station in items:
                return self._vex['TRACKS'][items[0]]["track_frame_format"]
        else:
            raise Hell

    def get_tracks(self, mode, station):
        """Return the name of the entry in the $TRACKS section for
MODE and STATION."""
        _mode = self._vex['MODE']
        for tracks in _mode[mode].getall('TRACKS'):
            if station in tracks[1:]:
                return tracks[0]
        else:
            raise Hell

    def get_sample_rate(self, mode, station):
        """Return the sample rate for MODE and STATION."""
        _freq = self._vex['FREQ']
        freq = self.get_freq(mode, station)
        sample_rate = _freq[freq]['sample_rate'].split()
        if sample_rate[1] in sample_rate_units:
            return float(sample_rate[0]) * sample_rate_units[sample_rate[1]]
        else:
            raise Hell

    def get_bits_per_sample(self, mode, station):
        """Return the number of bits per sample for MODE and STATION."""
        sign = 0
        mag = 0
        _tracks = self._vex['TRACKS']
        tracks = self.get_tracks(mode, station)
        for fanout in _tracks[tracks].getall('fanout_def'):
            if fanout[2] == 'sign':
                sign = 1
            if fanout[2] == 'mag':
                mag = 1
        return sign + mag

    def get_num_channels(self, mode, station):
        """Return the number of channels for MODE and STATION."""
        _freq = self._vex['FREQ']
        freq = self.get_freq(mode, station)
        return len(_freq[freq].getall('chan_def'))

def parse_vex_time(vex_time):
    return time.mktime(time.strptime(vex_time, "%Yy%jd%Hh%Mm%Ss"))

def format_vex_time(t): 
    return time.strftime("%Yy%jd%Hh%Mm%Ss", time.localtime(t))

import re
def parseFractionalTime(s):
    ms = re.search("\.(\d{4})s", s)
    if not ms is None:
        ms = float("0." + ms.group(1))
        s = re.sub("\.\d{4}","", s)
    else:
        ms = 0.0
    t = time.mktime(time.strptime(s, "%Yy%jd%Hh%Mm%Ss")) + ms
    return t
    
