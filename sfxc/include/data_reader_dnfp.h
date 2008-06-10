/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *  This file contains:
 *     - The declaration of a Data_reader_dnfp. This data reader
 *       mostly consist in glueing togther tcp and udp data reader
 *       with a very simple control protocol to request a specific
 *       file from a server.
 */
#ifndef DATA_READER_DNFP_H
#define DATA_READER_DNFP_H

#include "connexion.h"

#include "exception_common.h"
#include "data_reader_socket.h"
#include "data_writer_socket.h"
#include "data_reader_blocking.h"

/*******************************************************************************
* @class Data_reader_dnfp
* @author Damien Marchal
* @desc a data_reader and a control protocol to retreive in one line the content
* of a file stored on a server using the dnfp protocol.
*
*******************************************************************************/
class Data_reader_dnfp : public Data_reader {
public:

  // Create a data_reader_dnfp object connected to url
  // given in parameters.
  // the format for this kind of URL is:
  // dnfp://serverip:port/Filename?protocol=[tcp|udp]
  Data_reader_dnfp(pInterfaceIP interface, const std::string url);
  Data_reader_dnfp(const std::string url);
  ~Data_reader_dnfp(){};

  bool eof();
  bool can_read() {
    DEBUG_MSG("Data_reader_dnfp: can read not implemented");
    return true;
  }

private:
  void parse_url(const std::string url, String& server, String& port,
                String& filename);
  void connect(pInterfaceIP interface);
  void negociate();
  int do_get_bytes(size_t readb, char *buffer);


  String m_filename;
  String m_serverip;
  String m_serverport;
  String m_url;

  bool m_isEof;

  Data_writer_socket* m_writer;
  Data_reader_socket* m_reader;
  Data_reader_blocking* m_breader;
};


#endif // DATA_READER_DNFP_H
