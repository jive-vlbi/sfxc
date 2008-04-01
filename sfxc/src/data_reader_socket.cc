/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: DataWriter_socket.cc 278 2007-07-04 07:27:05Z kruithof $
 *
 */
#include <assert.h>

#include <iostream>

// defines send:
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <utils.h>

#include "data_reader_socket.h"
#include "connexion.h"

Data_reader_socket::Data_reader_socket(int socket)
{
    m_socket = socket;
    assert(m_socket > 0);
}

Data_reader_socket::Data_reader_socket(Connexion* connexion)
{
    m_socket = connexion->get_socket();
    assert(m_socket > 0);
}

Data_reader_socket::~Data_reader_socket() {

}


int Data_reader_socket::do_get_bytes(size_t nBytes, char *out) {
  assert(m_socket > 0);
  assert(out != NULL);
  return read(m_socket, (void *) out, nBytes);
}

bool Data_reader_socket::eof()
{
    return false;
}

bool Data_reader_socket::can_read() {
  DEBUG_MSG("Data_reader_socket: can read not implemented");
  return true;
}
