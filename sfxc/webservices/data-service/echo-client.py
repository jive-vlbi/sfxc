import sys

import jsonrpclib
jsonrpclib.config.version = 1.0

s = jsonrpclib.Server('http://head:8085/data', verbose=0)
print s.echo("Hello, World")
