/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Mark Kettenis <kettenis@jive.nl>, 2010
 *
 *  This file contains:
 *     - the definition of the Data_reader_mk5 object.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>

#include <algorithm>

#include "data_reader_mk5.h"

#define MK5READ_SOCKET "/tmp/mk5read"

struct mk5read_msg {
  char            vsn[9];
  uint64_t        off;
};

Data_reader_mk5::Data_reader_mk5(const std::string& url) {
  at_eof = false;

  // Parse URL.
  size_t vsn_end = url.rfind(":");
  std::string vsn = url.substr(6, vsn_end - 6);
  std::string offset = url.substr(vsn_end + 1);
  uint64_t off = ::strtoull(offset.c_str(), NULL, 0);

  fd = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (fd == -1) {
    at_eof = true;
    return;
  }

  struct sockaddr_un sun;
  sun.sun_family = AF_LOCAL;
  strncpy(sun.sun_path, MK5READ_SOCKET, sizeof(sun.sun_path));
  if (::connect(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
    at_eof = true;
    return;
  }

  struct mk5read_msg msg;
  strncpy(msg.vsn, vsn.c_str(), sizeof(msg.vsn));
  msg.off = off;
  if (::write(fd, &msg, sizeof(msg)) != sizeof(msg)) {
    at_eof = true;
    return;
  }
}

Data_reader_mk5::~Data_reader_mk5()
{
  if (fd != -1)
    close(fd);
}

bool
Data_reader_mk5::eof()
{
  return at_eof;
}

bool
Data_reader_mk5::can_read()
{
  return true;
}

size_t
Data_reader_mk5::do_get_bytes(size_t len, char *buf)
{
  char space[4096];
  ssize_t nbytes;

  if (buf == NULL) {
    len = std::min(len, sizeof(space));
    buf = space;
  }

  nbytes = ::read(fd, buf, len);
  if (nbytes > 0)
    return nbytes;

  at_eof = true;
  return 0;
}
