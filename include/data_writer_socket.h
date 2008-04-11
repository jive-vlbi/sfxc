/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 */
#ifndef Data_writer_socket_HH
#define Data_writer_socket_HH

#include <cassert>
#include "network.h"
#include "data_writer.h"

#include "utils.h"

class Data_writer_socket : public Data_writer {
public:
  Data_writer_socket(int socket);
  Data_writer_socket(Connexion* connexion);
  virtual ~Data_writer_socket();

  //unsigned int get_port();
  void flush();
  void close() {
    assert(false && "not implemented");
  }


  bool can_write() {
    DEBUG_MSG("can write not implemented");
    return true;
  }

protected:
  size_t do_put_bytes(size_t nBytes, char const*buff);
  int m_socket;
};

#endif // Data_writer_socket_HH
