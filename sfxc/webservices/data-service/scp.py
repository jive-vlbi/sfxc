#! /usr/bin/python

import os
import subprocess
import urlparse

urlparse.uses_relative.append('scp')
urlparse.uses_netloc.append('scp')
urlparse.uses_params.append('scp')
urlparse.clear_cache()

def scp(source, url, offset, nbytes):
    scheme = urlparse.urlparse(url).scheme
    netloc = urlparse.urlparse(url).netloc
    path = urlparse.urlparse(url).path
    args = ['ssh', netloc, 'scp', '-t', os.path.dirname(path)]
    proc = subprocess.Popen(args, stdin=subprocess.PIPE)

    if scheme != 'scp':
        raise Hell
    if not netloc:
        raise Hell
    if not path:
        raise Hell

    fp = open(source)
    stat = os.fstat(fp.fileno())
    if offset + nbytes > stat.st_size:
        nbytes = stat.st_size - offset
        pass

    print >>proc.stdin, "C0644 %d %s" % (nbytes, os.path.basename(path))

    while nbytes > 0:
        buf = fp.read(min(nbytes, 1024))
        proc.stdin.write(buf)
        nbytes -= len(buf)
        continue

    fp.close()
    proc.stdin.close()

    proc.wait()

    return
