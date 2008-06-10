/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 * This file contains a little file server based on a simple
 * protocol. The application use a config file named: dnfp_configfile.cfg
 * this file stores the list of file to expose to the clients...and there
 * symbolic name in the format:
 * name1 /path/to/real/file
 *
*/
#include <cassert>
#include <map>
#include <istream>
#include <cstdio>
#include "raiimutex.h"
#include "thread.h"
#include "data_writer_socket.h"
#include "data_reader_socket.h"
#include "data_reader_blocking.h"
#include "data_reader_file.h"
#include "connexion_listener_thread.h"
#include "monitor.h"
#include "exception_common.h"


#include <signal.h>
//void (*signal (int sig, void (*func)(int)))(int);

void signal_handler(int sig) {
  std::cout << "Crappy crappy !" << sig << " and " << SIGPIPE << std::endl;
  //exit(1);
}

class Server {
public:
  Server() {};
  virtual ~Server() {};

  void load_config_file(const std::string& filename) {
    // If this service is accepted then recode this thing...
    // A very nice security hole....
    char id[256];
    char file[256];
    FILE* fpt=fopen(filename.c_str(), "rt");
    if ( fpt == NULL ) MTHROW("Unable to open the config file ["+filename+"]");
    while ( !feof(fpt) ) {
      if ( fscanf(fpt, "%s %s", id, file) == 2 ) {
        std::cout << "Loading a new file mapping: ["<< id << "] -> [" << file << "]";
        if ( mapping_.count(id) == 0 ) {
          std::ifstream fs;
          fs.open( file, std::ifstream::in);
          if ( fs.is_open() ) {
            mapping_[std::string(id)]=std::string(file);
            std::cout << "\t\t[OK]" << std::endl;
          } else {
            std::cout << "\t\t[INVALID FILE NAME]" << std::endl;
          }
        }
      }

    }
    fclose(fpt);
  }

  bool can_stream(const std::string& id) {
    if ( mapping_.count(id) != 0) {
      return true;
    }
    return false;
  }

  std::string& get_filename(const std::string& id) {
    assert( mapping_.count(id) != 0 );
    return mapping_[id];
  }
private:
  std::map< String, String > mapping_;
};





/*****************************************
* @class Client_writer
* @desc This class implement the "file
* serving" thread. That stream the data
* to the client.
******************************************/
class Client_writer : public Thread {
public:
  Client_writer(uint32_t clientid, Data_writer* writer, Data_reader* reader, Server& server) :
      m_writer(writer), m_reader(reader), m_clientid(clientid), server_(server) {
    m_breader = new Data_reader_blocking( m_reader );
    m_buffersize = 10000000;
    m_buffer = new char[m_buffersize];
    m_monitor.set_name("server_monitor");
  }

  void send_file(const std::string& str) {
    Data_reader_file* filereader = new Data_reader_file("file://"+str);
    unsigned int packet=0;
    uint32_t bytetoread=m_buffersize;
    int32_t cmd=0;
    unsigned int counter =0;
    try {
      while ( !filereader->eof() ) {
        if ( counter == 0 ) m_monitor.begin_measure();
        //*m_breader >> cmd;
        if ( cmd == -1 ) {
          return;
        }
        //bytetoread = cmd;

        //std::cout << "Client ask for: " << bytetoread << std::endl;
        //assert(bytetoread <= m_buffersize);

        size_t byteread = filereader->get_bytes(bytetoread, m_buffer);
        counter += byteread;
        std::cout << "Number of byte read " << byteread << std::endl;
        //std::cout << "stream " << std::endl;
        while ( byteread != 0  ) {
          char tmp;
          int ret = m_writer->put_bytes(bytetoread, m_buffer);
          byteread -= ret;
        }

        //std::cout << "Couter " <<  counter << std::endl;

        //*m_writer << (uint32_t)filereader->eof();
        //std::cout << "finished" << std::endl;
        if (counter >= 10000000) {
          m_monitor.end_measure(counter);
          std::cout << "writing to client:"<< m_clientid<< " at speed:" << m_monitor.last_measure() << std::endl;
          counter = 0;
        }
      }
    } catch (...) {}
    std::cout << "THE FILE IS EOF" << std::endl;

  }

  void do_execute() {
    String cmd;
    std::cout << ": A client is connected..." << std::endl;
    *m_breader >> cmd;
    *m_writer << cmd;
    std::cout << ": Ho well [" << cmd << "]" << std::endl;

    std::string request;
    *m_breader >> request;
    std::cout << ": Client ask me for [" << request << "]" << std::endl;
    *m_writer << "OK";

    if ( server_.can_stream(request) ) {
      send_file( server_.get_filename(request) );
    } else {
      std::cerr << "Warning: a user requested an invalid file: [" << request << "]" << std::endl;
    }

    delete m_writer;
    delete m_reader;
    std::cout << "The client is disconnected !" << std::endl;
  }

private:
  Data_writer* m_writer;
  Data_reader* m_reader;
  Data_reader_blocking* m_breader;
  char* m_buffer;
  size_t m_buffersize;
  uint32_t m_clientid;
  QOS_MonitorSpeed m_monitor;
  Server& server_;
};

typedef Client_writer* Client_writer_ptr;

/****************************************
* @class Client_writer_allocator
* @desc This create a new client_writer
* for each incoming client.
******************************************/
class Client_writer_allocator : public Connexion_thread_allocator< Client_writer > {
  Server& server_;
  uint32_t m_counter;

  Mutex m_mutex;
public:
  Client_writer_allocator(Server& server) : server_(server) {
    m_counter= 0;
  }

  virtual ~Client_writer_allocator() {}

  Client_writer* allocate(int socket) {
    RAIIMutex rc(m_mutex);

    Data_writer_socket* writer = new Data_writer_socket( socket );
    Data_reader_socket* reader = new Data_reader_socket( socket );
    Client_writer_ptr element = new Client_writer(m_counter++, writer, reader, server_);
    return element;
  }
};

typedef Client_writer_allocator* Client_writer_allocator_ptr;
typedef Connexion_listener_thread< Client_writer > Client_writer_listener;
typedef Client_writer_listener* Client_writer_listener_ptr;

void print_usage(char* appname) {
  std::cerr << "Syntax: "<< appname << "  <--if ip.to.listen>     <--port port to listen(default 2630)>" << std::endl;
}

/* Server for the SC07 demo */
int main(int argc, char** argv) {
  try {
    signal(SIGPIPE, signal_handler);
    InterfaceIP* interface=Network::get_any_interface();
    int port=2630;
    bool arg_problem=false;
    for (int i=1;i<argc;i++) {
      if ( !strcmp(argv[i],"--if") ) {
        if ( ++i < argc ) {
          interface= Network::get_interface_by_name(argv[i]);
        } else arg_problem= true;
      } else if ( !strcmp(argv[i],"--port") ) {
        if ( ++i < argc )
          port=atoi(argv[i]);
        else arg_problem= true;
      } else if ( !strcmp(argv[i], "--help") ) {
        print_usage(argv[0]);
        exit(0);
      } else {
        arg_problem= true;
      }

      if (arg_problem) {
        if ( i < argc )
          std::cerr << "Invalid argument: [" << argv[i] << "]" << std::endl;
        print_usage(argv[0]);
        exit(1);
      }
    }



    if (interface == NULL) {
      std::cerr << "Unable  to find a valid interface !" <<std::endl;
      std::vector<InterfaceIP*> intf;
      Network::get_interfaces(intf);
      std::cerr << "valid IPV4 interfaces are:" << std::endl;
      for (int i=0;i<intf.size();i++) {
        std::cerr << "\t" << intf[i]->name() << "(" << intf[i]->ip() << ")" << std::endl;
      }
      exit(2);
    }

    printf("PORT %d %d\n", port, interface);
    Connexion_listener_ptr listener = interface->create_listener( port );
    printf("PORT%d %d\n", port, listener);

    if ( listener == NULL ) {
      MTHROW("Strange exception");
    }
    std::cout << "A dnfp fileserver is listening at: " << listener << std::endl;

    Server server;
    server.load_config_file("dnfp_configfile.cfg");
    Client_writer_allocator_ptr clientallocator = new Client_writer_allocator(server);
    Client_writer_listener_ptr  clientlistener  = new Client_writer_listener(listener,       clientallocator);
    clientlistener->start();

    ThreadPool::s_wait_for_all_termination();
  } catch (Exception &e) {
    std::cerr << e << std::endl;
  }
}

