/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 * This file contains a little file server based on a simple control
 * protocol. The application use a config file named: dnfp_configfile.cfg
 * this file stores the list of file to expose to the clients...and there
 * symbolic name in the format:
 * name1 /path/to/real/file
 *
 * This file contains:
 *   the complete implementation of the fileserver. It is composed of three
 *   C++ class:
 *     - Server
 *     - Client_writer (inherit from thread).
 *          - Reading_thread (inherit from thread).
 *          - Streaming_thread (inherit from thread).
 *   The implementation look a bit Java styled :)
 */
#include <cassert>
#include <map>
#include <istream>
#include <cstdio>
#include <sys/socket.h>
#include <signal.h>
#include "raiimutex.h"
#include "thread.h"
#include "data_writer_socket.h"
#include "data_reader_socket.h"
#include "data_reader_blocking.h"
#include "data_reader_file.h"
#include "data_writer_udp.h"
#include "connexion_listener_thread.h"
#include "monitor.h"
#include "exception_common.h"
#include "threadsafe_queue.h"
#include "memory_pool.h"
#include "memory_pool_elements.h"

void signal_handler(int sig) {
  //std::cout << "Crappy crappy !" << sig << " and " << SIGPIPE << std::endl;
}

/*******************************************************************************
* @class Server
* @author Damien Marchal
* @desc A basic class that implement the main server functionnality.
* A basic class that implement the main server functionnality:
*     - load and parse a config file.
*     - listen for incoming connexion.
*     - create clients threads for each incoming connexion.
*******************************************************************************/
class Server {
public:
  Server() {};
  virtual ~Server() {};

  /*****************************************************************************
  * @desc Load a given configuration file.
  * @param const std::string& Filename the name of the configuration file.
  * The configuration file is a mapping between a name and the path to the
  * associated file. The mapping is simply a list of pairs
  * name /path/to/the/file.
  * The name and path entry should be < 256 chars each.
  *****************************************************************************/
  void load_config_file(const std::string& filename) {
    // If this service is accepted then recode this thing...
    // A very nice security hole....
    char id[256];
    char file[256];
    FILE* fpt=fopen(filename.c_str(), "rt");

    // Check if the file was successfully opened. If not
    // throw and Exception.
    if ( fpt == NULL ) MTHROW("Unable to open the config file ["+filename+"]");
    while ( !feof(fpt) ) {
      // Scan one line of the input file.
      if ( fscanf(fpt, "%s %s", id, file) == 2 ) {
        // If two entries are find check if this correspond
        std::cout << "Loading a new file mapping: ["<< id << "] -> [" << file << "]";

        // Insure that this ID is not already used for a different
        // file.
        if ( mapping_.count(id) == 0 ) {
          // Check if the filepath is valid by opening it
          // and checking it isuccessfully opened.
          std::ifstream fs;
          fs.open( file, std::ifstream::in);
          if ( fs.is_open() ) {
            // If the file successfully open then we can add
            // the entry into the list of file to serve.
            mapping_[std::string(id)]=std::string(file);
            std::cout << "\t[OK]" << std::endl;
          } else {
            // If the entry is invali we print some error message
            // and cancel this entry.
            std::cout << "\t[INVALID FILE NAME]" << std::endl;
          }
        } else {
          // As said in the text of the exception.
          MTHROW(String("The file id [")+id+"] is already in use.");
        }
      }
    }
    // Simply closing the file.
    fclose(fpt);
  }

  /*****************************************************************************
  * @desc Check wheter a name correspond to a file that can be strem.
  *****************************************************************************/
  bool can_stream(const std::string& id) {
    if ( mapping_.count(id) != 0) {
      return true;
    }
    return false;
  }

  /*****************************************************************************
  * @desc return the filename corresponding to the given ID.
  *****************************************************************************/
  std::string& get_filename(const std::string& id) {
    assert( mapping_.count(id) != 0 );
    return mapping_[id];
  }
private:
  std::map< String, String > mapping_;
};

/*******************************************************************************
* @class Client_writer
* @desc This class implement the "file serving" thread. That stream the data
* to the client.
*******************************************************************************/
#define CHUNK_SIZE 10000000
class Client_writer : public Thread {
  typedef Memory_pool_fixed_size_element<char, CHUNK_SIZE>        chunk_type;
  typedef Memory_pool< chunk_type >::value_type                   pool_type;
  typedef Threadsafe_queue< pool_type >::value_type               queue_type;

public:
  Client_writer(uint32_t clientid, int socket, Server& server) :
      m_clientid(clientid),
      socket_(socket),
      server_(server),
      pool_(10, NO_RESIZE ) {
    m_writer = new Data_writer_socket(socket_);
    m_reader = new Data_reader_socket(socket_),
    m_breader = new Data_reader_blocking( m_reader );
    udp_writer_ = NULL;
  }

  virtual ~Client_writer() {
    delete m_writer;
    delete m_reader;
    shutdown(socket_, SHUT_RDWR);
    std::cout << " ["<<  m_clientid << "]: Finalized " << std::endl;
  }

  /*****************************************************************************
  * @desc Kill the sub-thread and wait their termination before to leave the
  * function.
  *****************************************************************************/
  void kill(){
    queue_.close();
  }

  /*****************************************************************************
  * @desc Start streaming a file to the given client.
  *****************************************************************************/
  void send_file(Data_reader& reader, Data_writer& writer) {
    // creating the reading thread.
    Reading_thread reading_thread(m_clientid, reader, pool_, queue_, threadpool);

    // and its corresponding sending thread.
    Sending_thread sending_thread(m_clientid, writer, queue_, threadpool);

    // Start the two threads and wait and block until their
    // termination.
    threadpool.register_thread( reading_thread.start() );
    threadpool.register_thread( sending_thread.start() );

    // Wait that both thread terminates.
    wait( threadpool );
  }

  /*****************************************************************************
  * @desc Call by the super-class Thread. Process the user-request and start
  * serving files if needed.
  *****************************************************************************/
  void do_execute() {
    String cmd, protocol, clientaddress, clientport;
    std::cout << ": A client is connected..." << std::endl;

    /// Client handshaking function.
    *m_breader >> cmd;
    *m_writer << cmd;
    std::cout << " ["<<  m_clientid << "]: Ho well [" << cmd << "]" << std::endl;

    /// Request from the client.
    std::string request;
    *m_breader >> request;
    std::cout << " [" <<  m_clientid << "]: Client ask me for [" << request << "]" << std::endl;

    /// Protocol requested from the client.
    *m_breader >> protocol;
    std::cout << " ["<<  m_clientid << "]: PROTOCOL IS: " << protocol <<  std::endl;

    /// If the protocol is UDP we have more option to retreive:
    if ( protocol == "udp") {
      /// The clientaddress on which he is listening
      *m_breader >> clientaddress;
      std::cout << " ["<<  m_clientid << "]: ADDRESS IS: " << clientaddress <<  std::endl;

      /// The clientport on which he is listening for packet
      *m_breader >> clientport;
      std::cout << " ["<<  m_clientid << "]: PORT: " << clientport << std::endl;

      /// We can create the connexion and the data_writer object.
      Connexion* cnx = Network::connect_to(clientaddress, atoi( clientport.c_str() ), SOCK_DGRAM);
      udp_writer_ = new Data_writer_udp( cnx->get_socket()  );
    }

    /// Check that the file requested is a valid file.
    if ( server_.can_stream(request) ) {
      /// If so, tell it to the client.
      *m_writer << "OK";

      /// Create the data-reader/writer to the client and start streaming the
      /// file
      Data_reader_file filereader("file://"+server_.get_filename(request));
      if ( udp_writer_ )
        send_file( filereader, *udp_writer_);
      else
        send_file( filereader, *m_writer);
    } else {
      /// If the file is not valid, tell it ot client and print and info msg.
      *m_writer << "NOT-OK";
      std::cerr << " ["<<  m_clientid << "]: Warning: a user requested an invalid file: [" << request << "]" << std::endl;
    }
    std::cout << " ["<<  m_clientid << "]: Finalizing servicing thread." << std::endl;

    /// The file is streamed or a problem arose. Shutdown the socket
    shutdown(socket_, SHUT_RDWR);

    /// Here we leave the thread, so we redefined our state.
    isrunning_ = false;
  }

private:
  /// Unique client ID.
  uint32_t m_clientid;

  /// Pool of thread (one for reading, one for stream to client)
  ThreadPool threadpool;

  int socket_;
  Data_writer* m_writer;
  Data_reader* m_reader;
  Data_reader_blocking* m_breader;
  Data_writer_udp* udp_writer_;

  /// Server data-structure, shared by all client writer and used to indicate
  /// which files are allowed to be streamed.
  Server& server_;

  /// Memory_pool and its queue connecting the two processing thread. The thread
  /// stops if the queue is closed.
  Memory_pool< chunk_type > pool_;
  Threadsafe_queue< pool_type > queue_;

  class Reading_thread : public Thread {
    Data_reader& reader_;
    Memory_pool<chunk_type>& mpool_;
    Threadsafe_queue<pool_type>& output_;
    ThreadPool& threadpool_;
    uint32_t m_clientid;
  public:
    Reading_thread(uint32_t clientid, Data_reader& reader,
                   Memory_pool<chunk_type>& pool,
                   Threadsafe_queue<pool_type>& output,
                   ThreadPool& threadpool):
        m_clientid(clientid),
        reader_(reader), mpool_(pool), output_(output),
        threadpool_(threadpool) {}

    // implementing a threaded function.
    void do_execute() {
      static int toto=0;
      uint64_t totalsend=0;
      // isrunning_ variable is inherited from the Thread parent class
      uint64_t begin,end;
      getusec(begin);
      pool_type element;
      try {

        while ( isrunning_ && !output_.isclose() && !reader_.eof() ) {
          // allocate a new chunk of data from the memory_pool.
          // this function should block if no elements are available.
          element = mpool_.allocate();

          // reading from the file.
          uint64_t byteread = reader_.get_bytes( element.data().size(), element.data().buffer() );
          totalsend+=byteread;

          // sending the read packet to the other thread.
          output_.push( element );
        }
        output_.close();
      } catch (...) {
        //std::cout << " ["<<  m_clientid << "]: Closed by exception " << std::endl;
      }
      getusec(end);
      std::cout << " ["<<  m_clientid << "]: reading speed: " << (1.0*totalsend/(1024.0*1024.0)) / tickToSec(end-begin);
      std::cout << "MB/s"<< std::endl;
      std::cout << " ["<<  m_clientid << "]: Reading terminates" << std::endl;
    }
  };

class Sending_thread : public Thread {
    Threadsafe_queue<pool_type>& input_;
    Data_writer& writer_;
    ThreadPool& threadpool_;
    uint32_t m_clientid;
  public:
    Sending_thread(uint32_t clientid, Data_writer& writer, Threadsafe_queue<pool_type>& input,
                   ThreadPool& threadpool):
        m_clientid(clientid),
        writer_(writer), input_(input),
        threadpool_(threadpool) {}

    void do_execute() {
      // isrunning_ variable is inherited from the Thread parent class
      try {
        pool_type element;
        while ( isrunning_  && !input_.isclose() ) {
          element = input_.front();
          writer_.put_bytes( element.data().size(),
                             element.data().buffer() );
          input_.pop();
        }
        std::cout << " ["<<  m_clientid << "]: Streaming terminates" << std::endl;
        input_.close();
      } catch (...) {
        //std::cout << " ["<<  m_clientid << "]: Exception detected" << std::endl;
      }
      if ( !input_.isclose() ) input_.close();
    }
  };
};

typedef Client_writer* Client_writer_ptr;

/*******************************************************************************
* @class Client_writer_allocator
* @desc This create a new client_writer
*       for each incoming client.
*******************************************************************************/
class Client_writer_allocator : public Connexion_thread_allocator< Client_writer > {
  Server& server_;
  uint32_t m_counter;
  std::vector<Client_writer_ptr> vector_clients_;
  Mutex m_mutex;

  /// ThreadPool client_thread_pool;
public:
  Client_writer_allocator(Server& server) : server_(server) {
    m_counter= 0;
  }

  virtual ~Client_writer_allocator() {
    for (int i=0;i<vector_clients_.size();i++) {
        vector_clients_[i]->kill();
    }

    /// This is pretty ugly And MUST be fixed.
    for (int i=0;i<vector_clients_.size();i++) {
        while( vector_clients_[i]->isrunning() );
        delete vector_clients_[i];
    }
  }

  /*****************************************************************************
  * @desc This function is called by the Client_thread_allocator object to
  * allocate a new Client_writer object for each incoming connexion. This object
  * also keep a track of active connexions.
  *****************************************************************************/
  Client_writer* allocate(int socket) {
    RAIIMutex rc(m_mutex);

    /// Create a new client_writer to handle the client request.
    Client_writer_ptr element = new Client_writer(m_counter++, socket, server_);

    /// This is buggy the thread pool will not like some object to be
    /// client_thread_pool.register_thread();

    /// Check if some clients became inactive.
    for (int i=0;i<vector_clients_.size();i++) {
      if ( !vector_clients_[i]->isrunning() ) {
        delete vector_clients_[i];
        vector_clients_[i] = element;
        return element;
      }
    }
    vector_clients_.push_back(element);
    return element;
  }
};

typedef Client_writer_allocator* Client_writer_allocator_ptr;
typedef Connexion_listener_thread< Client_writer > Client_writer_listener;
typedef Client_writer_listener* Client_writer_listener_ptr;

void print_usage(char* appname) {
  std::cerr << "Syntax: "<< appname << "  [--if listeningip(default=any)] "
  << " [--port listeningport(default=2630)]" << std::endl;
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

    Connexion_listener_ptr listener = interface->create_listener( port );

    if ( listener == NULL ) {
      MTHROW("Unable to create a listener");
    }

    Server server;

    // Load the list of file to serve and their symbolic names.
    server.load_config_file("dnfp_configfile.cfg");

    Client_writer_allocator_ptr clientallocator = new Client_writer_allocator(server);
    Client_writer_listener_ptr  clientlistener  = new Client_writer_listener(listener,
        clientallocator);

    // Start the listening thread.
    clientlistener->start();

    // Wait infinitely
    std::cout << "A dnfp fileserver is listening at: " << listener << std::endl;
    ThreadPool::s_wait_for_all_termination();
  } catch (Exception &e) {
    std::cerr << e << std::endl;
  }
}

