/* Copyright (c) 2010-2012 Joint Institute for VLBI in Europe (Netherlands)
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <cstdlib>

#include "data_reader_mk5.h"

#define MK5READ_SOCKET "/tmp/mk5read"

struct mk5read_msg {
  char            vsn[9];
  char            pad[7];
  uint64_t        off;
};

Data_reader_mk5::Data_reader_mk5(const std::string& url) {
  at_eof = true;

  // Parse URL.

  size_t prot_end = url.find("://");
  size_t host_start = prot_end + 3;

  if (prot_end == std::string::npos)
    return;

  size_t path_start = url.find("/", host_start);
  size_t sock_end = url.rfind("/");
  size_t vsn_start = sock_end + 1;
  size_t vsn_end = url.rfind(":");
  size_t off_start = vsn_end + 1;

  std::string prot = url.substr(0, prot_end);
  std::string host;
  if (host_start != path_start && host_start != vsn_start)
    host = url.substr(host_start, path_start - host_start);
  std::string sock = MK5READ_SOCKET;
  if (path_start != std::string::npos && path_start != sock_end)
    sock = url.substr(path_start, sock_end - path_start);
  std::string vsn = url.substr(vsn_start, vsn_end - vsn_start);
  uint64_t off = ::strtoull(url.substr(off_start).c_str(), NULL, 0);

  if (host.empty()) {
    fd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
    if (fd == -1)
      return;

    struct sockaddr_un sun;
    sun.sun_family = AF_LOCAL;
    std::strncpy(sun.sun_path, sock.c_str(), sizeof(sun.sun_path));
    if (::connect(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
      ::close(fd);
      return;
    }
  } else {
    size_t host_end = host.find(":");
    std::string port = "8888";
    if (host_end != std::string::npos)
      port = host.substr(host_end + 1);
    host = host.substr(0, host_end);

    struct addrinfo hints, *res0;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (::getaddrinfo(host.c_str(), port.c_str(), &hints, &res0))
      return;

    if (res0 == NULL)
      return;

    fd = ::socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
    if (fd == -1) {
      ::freeaddrinfo(res0);
      return;
    }

    if (::connect(fd, res0->ai_addr, res0->ai_addrlen) == -1) {
      ::close(fd);
      ::freeaddrinfo(res0);
      return;
    }
  }

  struct mk5read_msg msg;
  strncpy(msg.vsn, vsn.c_str(), sizeof(msg.vsn));
  msg.off = off;
  if (::write(fd, &msg, sizeof(msg)) != sizeof(msg))
    return;

  at_eof = false;
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
