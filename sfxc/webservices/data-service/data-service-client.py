import sys

import jsonrpclib
jsonrpclib.config.version = 1.0

s = jsonrpclib.Server('http://sfxc-g0:8080/data', verbose=0)
print s.transfer_data(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]),
                      "scp://nexpres@sfxc-h0/scratch/nexpres",
#                     "http://head.sfxc:8080/broker")
                      None)
