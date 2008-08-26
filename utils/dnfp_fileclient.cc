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
#ifdef USE_MPI
#undef USE_MPI
#endif

#include <cassert>
#include <iostream>
#include "thread.h"
#include "data_writer_socket.h"
#include "data_writer_file.h"
#include "data_reader_socket.h"
#include "data_reader_blocking.h"
#include "data_reader_dnfp.h"
#include "connexion_listener_thread.h"
#include "monitor.h"
#include "exception_common.h"
#include "rttimer.h"

bool use_pattern_global_ = false;

class Client_reader  : public Thread {
public:
#define CHUNK_SIZE 8000
  Client_reader(Data_reader_dnfp* reader) {
    m_buffersize =CHUNK_SIZE;
    m_buffer = new char[m_buffersize];
    m_reader = reader;
  }

  void do_execute() {
    RTTimer timer;
    uint64_t totalread=0;
    uint64_t begin,end;
    int state=0;
    double lasttime=0;
    uint64_t totalwrite=0;
    Data_writer_file dwf("file:///home/damien/SFXCDATA/out.data");
    try {
      timer.start();
      while ( !m_reader->eof() ) {
        uint64_t val = m_reader->get_bytes(m_buffersize, m_buffer);
        dwf.put_bytes(val, m_buffer);
        totalwrite +=val;

        if( use_pattern_global_ ){
         for(unsigned int i=0;i<m_buffersize;i++){
          if( m_buffer[i] != i%128){
                std::cout << "["<< i<<": " << (int)m_buffer[i] << "]"  << std::endl;
          }
         }
        }

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

    uint64_t val = m_reader->get_bytes(m_buffersize, m_buffer);
    dwf.put_bytes(val, m_buffer);
    totalwrite +=val;

    std::cout << "EOF: " << totalwrite << std::endl;
  }

  typedef Client_reader* Client_reader_ptr;

private:
  Data_reader* m_reader;
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

