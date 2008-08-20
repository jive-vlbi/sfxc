/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 * This file contains a little file client based on a simple
 * protocol. The application simply connect to a dnfp server
 * and request a file.
 */
#include <cassert>
#include <iostream>
#include "thread.h"
#include "data_writer_socket.h"
#include "data_reader_socket.h"
#include "data_reader_blocking.h"
#include "data_reader_dnfp.h"
#include "connexion_listener_thread.h"
#include "monitor.h"
#include "exception_common.h"
#include "rttimer.h"

class Client_reader  : public Thread {
public:
  Client_reader(Data_reader_dnfp* reader) {
    m_buffersize =1000000;
    m_buffer = new char[m_buffersize];
    m_breader = new Data_reader_blocking(reader);
  }

  void do_execute() {
    RTTimer timer;
    uint64_t totalread=0;
    uint64_t begin,end;
    int state=0;
    double lasttime=0;
    try {
      timer.start();
      while ( !m_breader->eof() ) {
        int val = m_breader->get_bytes(m_buffersize, m_buffer);
        totalread += val;
        if ( timer.measured_time()-lasttime >= 1 ) {
          std::cout << " read:" << 1.0*toMB(totalread) << "MB, speed:"
          << 1.0*toMB(totalread)/timer.measured_time()
          << "MB/sec" << std::endl;
          lasttime = timer.measured_time();
        }

      }
    } catch (Exception& e) {
      std::cerr << e << std::endl;
    }

    std::cout << "EOF" << std::endl;
  }

  typedef Client_reader* Client_reader_ptr;

private:
  Data_reader* m_breader;
  char* m_buffer;
  unsigned int m_buffersize;
  QOS_MonitorSpeed m_monitor;
};


void print_usage(char* appname) {
  std::cerr << "Syntax: "<< appname << "  dnfp://<host>:<port>/<fileid>" << std::endl;
}

int main(int argc, char** argv) {
  try {
    InterfaceIP* interface = Network::get_any_interface();

    int port=2630;
    if ( argc != 2 ) {
      print_usage(argv[0]);
      exit(1);
    }

    String url=String(argv[1]);
    Data_reader_dnfp dnf(interface, url);
    Client_reader reader(&dnf);
    reader.start();
    ThreadPool::s_wait_for_all_termination();
  } catch (Exception& ex) {
    std::cout << ex << std::endl;
  }
}

