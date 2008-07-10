/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - xxx
 * This file contains:
 *   - xxx
 */
#ifndef DATA_READER_SOCKET_HH
#define DATA_READER_SOCKET_HH

#include "network.h"
#include "data_reader.h"
#include "utils.h"

class Data_reader_socket : public Data_reader {
public:
  Data_reader_socket(int socket);
  Data_reader_socket(Connexion* connexion);

  virtual ~Data_reader_socket();
  virtual bool eof();
  void closef() {
    SFXC_ASSERT_MSG(false,
                    "Function not implemented");
  }

  bool can_read();

  int get_fd() {
    return m_socket;
  }

protected:
  int do_get_bytes(size_t nBytes, char *buff);

  int m_socket;

private:
};

#endif // DATAREADER_SOCKET_HH
