/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "data_reader_tcp.h"
#include "utils.h"

#include "tcp_connection.h"

// for inet_ntoa (debugging):
#include <arpa/inet.h>

#include <sys/poll.h>
#include <errno.h>

Data_reader_tcp::Data_reader_tcp(uint64_t *ip_addr, int nAddr, unsigned short int port)
    : Data_reader(), socket(-1) {
  TCP_Connection connection(true);
  int i=0;
  do {
    i = (i+1)%nAddr;
    socket = connection.do_connect(ip_addr[i], port);
  } while (socket <= 0);

  SFXC_ASSERT(socket > 0);
}

Data_reader_tcp::~Data_reader_tcp() {
  if (socket > 0) close(socket);
}

int Data_reader_tcp::do_get_bytes(size_t nBytes, char*out) {
  SFXC_ASSERT(socket > 0);

  if (out == NULL) {
    size_t buff_size = 1000000;
    buff_size = (nBytes < buff_size ? nBytes : buff_size);
    char buff[(int)buff_size];
    return read(socket, (void *) buff, buff_size);
  }

  /* Read data from socket */
  return read(socket, (void *) out, nBytes);
}

unsigned int Data_reader_tcp::get_port() {
  return port;
}


bool Data_reader_tcp::can_read() {
  //     int fd;           /* file descriptor */
  //     short events;     /* requested events */
  //     short revents;    /* returned events */
  //   };

  pollfd fds[1];
  fds[0].fd = socket;
  fds[0].events = POLLIN;

  int ret = poll(fds, 1, /*timeout in miliseconds*/ 0);
  if (ret > 0) {
    return ((fds[0].revents & POLLIN) != 0);
  }

  return false;
}

