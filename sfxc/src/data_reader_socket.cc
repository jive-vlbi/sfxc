/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: DataWriter_socket.cc 278 2007-07-04 07:27:05Z kruithof $
 *
 */

#include "data_reader_socket.h"
#include "connexion.h"
#include "utils.h"

#include <iostream>

// defines send:
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <utils.h>

#include <sys/poll.h>
#include <errno.h>

Data_reader_socket::Data_reader_socket(int socket) {
  m_socket = socket;
  SFXC_ASSERT(m_socket > 0);
}

Data_reader_socket::Data_reader_socket(Connexion* connexion) {
  m_socket = connexion->get_socket();
  SFXC_ASSERT(m_socket > 0);
}

Data_reader_socket::~Data_reader_socket() {}


int Data_reader_socket::do_get_bytes(size_t nBytes, char *out) {
  SFXC_ASSERT(m_socket > 0);
  SFXC_ASSERT(out != NULL);
  return read(m_socket, (void *) out, nBytes);
}

bool Data_reader_socket::eof() {
  return false;
}

bool Data_reader_socket::can_read() {
	//     int fd;           /* file descriptor */
  //     short events;     /* requested events */
  //     short revents;    /* returned events */
  //   };

  pollfd fds[1];
  fds[0].fd = m_socket;
  fds[0].events = POLLIN;

  int ret = poll(fds, 1, /*timeout in miliseconds*/ 0);
  if (ret > 0) {
    return ((fds[0].revents & POLLIN) != 0);
  }

  return false;
}
