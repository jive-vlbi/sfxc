from mark5 import *

fn = ("/sfxc/data/N08C1/n08c1_ef_no0033")
mk5 = Mark5ScanHandler(fn)

times = []
poses = []
for i in range(20):
    poses.append(mk5.f.tell())
    dt = mk5.extractDatetime()
    times.append(dt)
    try:
        mk5.skipData()
    except EOFError:
        break
for t, p in zip(times, poses):
    t1 = mk5.bytes_starting_position(p)
    print t, p, t1
size = 40*1024
start = 1024
fn = "chunk1"
mk5.get_chunks("chunk1", 4096, size, start)
f.seek(start)
s1 = f.read(size)
f2 = open(fn)
s2 = f2.read(size)
if s1!=s2:
    raise RuntimeError, "chunking fails"
