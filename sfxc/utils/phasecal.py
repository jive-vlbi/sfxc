#! /usr/bin/python

import math
import struct
import sys

import numpy as np
import scipy as sp
import pylab as p
from scipy import fftpack, signal

fp = open(sys.argv[1], 'r')
fp.read(68)
fp.read(7 * (20 + 1600 * 4))

header = fp.read(20)
header = struct.unpack("BBBBIIII", header)
print header
num_samples = header[7]
buf = fp.read(num_samples * 4)
dd = np.frombuffer(buf, dtype='int32').astype(float)
ddx = dd.max()
ddp = dd / ddx

dmvec = np.exp(2 * math.pi * 1j * 510e3/16e6 * np.array(range(num_samples)))

#p.plot(ddp, color="red")
#p.plot(dmvec.real, color="blue")
#p.plot(dmvec.imag, color="green")
#p.show()

sdd = sp.fftpack.ifft(dd)
sddh = np.concatenate((sdd[0:num_samples / 2], np.zeros(num_samples / 2)))
dda = sp.fftpack.fft(sddh)
#dda = sp.fftpack.hilbert(dd)

ddc = dda * dmvec

#p.plot(ddc.real, color="red")
#p.plot(ddc.imag, color="blue")
#p.show()

pp = np.sum(ddc.reshape((100, -1)), axis=0)

#p.plot(pp.real, color="red")
#p.plot(pp.imag, color="blue")
#p.plot(np.abs(pp), color="black")
#p.show()

spp = sp.fftpack.ifft(pp)
abp = np.abs(spp)
xabp = np.max(abp)
abp = abp / xabp

pbp = np.angle(spp) / (2 * math.pi)

p.plot(abp, color="red")
p.plot(pbp, color="blue")
p.show()
