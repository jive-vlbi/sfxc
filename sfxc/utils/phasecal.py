#! /usr/bin/python

import math
import struct
import sys

import numpy as np
import scipy as sp
import pylab as p
from scipy import fftpack, signal

# JIVE Python modules
from vex import Vex

vex = Vex(sys.argv[1])
for freq in vex['FREQ']:
    sample_rate = vex['FREQ'][freq]['sample_rate']
    if sample_rate.split()[1] == "Ms/sec":
        sample_rate = float(sample_rate.split()[0]) * 1e6
        break
    break
for freq in vex['FREQ']:
    chan_def = vex['FREQ'][freq]['chan_def']
    if chan_def[1].split()[1] == "MHz":
        freq = float(chan_def[1].split()[0]) * 1e6
        break
    break

offset = (int(freq + 999999) / 1000000) * 1e6 - freq

in_fp = open(sys.argv[2], 'r')
header = in_fp.read(68)

out_fp = open(sys.argv[3], 'w')
out_fp.write(header)

while in_fp:
    header = in_fp.read(20)
    if len(header) == 0:
        break
    header = struct.unpack("BBBBIIII", header)
    sideband = header[2]
    sign = -1 if sideband == 0 else 1
    num_samples = header[7]
    buf = in_fp.read(num_samples * 4)
    dd = np.frombuffer(buf, dtype='int32').astype(float)

    # Filter out vectors with no valid data
    ddx = dd.max()
    if ddx == 0:
        continue

    dmvec = np.exp((sign * 2 * math.pi * 1j * offset / sample_rate) *
                   np.array(range(num_samples)))

    sdd = sp.fftpack.ifft(dd)
    sddh = np.concatenate((sdd[0:num_samples / 2], np.zeros(num_samples / 2)))
    dda = sp.fftpack.fft(sddh)

    ddc = dda * dmvec

    pp = np.sum(ddc.reshape((100, -1)), axis=0)

    header = list(header)
    header[7] = len(pp)
    header = struct.pack("BBBBIIII", *header)
    out_fp.write(header)
    buf = np.getbuffer(pp.astype('complex64'))
    out_fp.write(buf)
    continue
