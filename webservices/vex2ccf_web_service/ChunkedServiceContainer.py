# Copyright (C) 2007 Joint Institute for VLBI in Europe
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from ZSI.ServiceContainer import SOAPRequestHandler, ServiceContainer
from ZSI import Fault

class ChunkedSOAPRequestHandler(SOAPRequestHandler):
    """Replacement for the ZSI SOAPRequesthandler that can handle the
    HTTP/1.1 chunked transfer encoding."""

    def do_POST(self):
        """The POST command."""

        chunked = False

        # If the headers indicate chunked transfer encoding, fake
        # a Content-Length header field that matches the first chunk.
        try:
            te = self.headers['transfer-encoding']
            if te.startswith('chunked'):
                chunked = True
                length = int(self.rfile.readline(), 16)
                if length > 0:
                    self.headers['content-length'] = length
                    pass
                pass
            pass
        except:
            pass

        # Process the request.
        SOAPRequestHandler.do_POST(self)

        # If we're using chunked transfer encoding, check for the last chunk.
        if chunked:
            crlf = self.rfile.readline()
            length = int(self.rfile.readline(), 16)
            if not length == 0:
                self.send_fault(Fault(500, "Cannot handle multiple chunks"))
                pass
            pass
        pass


def AsServer(port=80, services=()):
    address = ('', port)
    sc = ServiceContainer(address, services, ChunkedSOAPRequestHandler)
    sc.serve_forever()
