#! /usr/bin/python

import sys

from decoder import Mark5B, time2vex

class MyMark5B(Mark5B):
    def sync(self, src):
        pos = 0
        fp = open(src)
        buf = fp.read(self.frame_size + self.sync_size)
        while len(buf) > 0:
            # Search for sync word.
            offset = buf.find('\xed\xde\xad\xab')
            if offset == -1:
                pos += len(buf) - self.sync_size
                buf = buf[-self.sync_size:]
                buf += fp.read(self.frame_size)
                continue

            # Found sync word.  Skip it and make sure we have a full header in
            # the remaining buffer.
            pos += offset + self.sync_size
            buf = buf[offset + self.sync_size:]
            if len(buf) < self.header_size:
                buf += fp.read(self.header_size - len(buf))
                pass

            # If we failed to read a full header, punt.
            if len(buf) < self.header_size:
                break

            secs = self.decode_header(buf)
            print time2vex(secs)
            break;

        return

d = MyMark5B()
d.sync(sys.argv[1])
