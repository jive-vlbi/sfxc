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
#ifdef USE_MPI
#undef USE_MPI
#endif

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
#include "rttimer.h"

void signal_handler(int sig)
{
}


#define CHUNK_SIZE 10000000
#define NETWORK_PACKET 1448

bool real_read_global_=true;
bool use_pattern_global_=false;


/*******************************************************************************
* @class Server
* @author Damien Marchal
* @desc A basic class that implement the main server functionnality.
* A basic class that implement the main server functionnality:
*     - load and parse a config file.
*     - listen for incoming connexion.
*     - create clients threads for each incoming connexion.
*******************************************************************************/
class Server
{
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
    void load_config_file(const std::string& filename)
    {
        // If this service is accepted then recode this thing...
        // A very nice security hole....
        char id[256];
        char file[256];
        FILE* fpt=fopen(filename.c_str(), "rt");

        // Check if the file was successfully opened. If not
        // throw and Exception.
        if ( fpt == NULL ) MTHROW("Unable to open the config file ["+filename+"]");
        while ( !feof(fpt) )
        {
            // Scan one line of the input file.
            if ( fscanf(fpt, "%s %s", id, file) == 2 )
            {
                // If two entries are find check if this correspond
                std::cout << "Loading a new file mapping: ["<< id << "] -> [" << file << "]";

                // Insure that this ID is not already used for a different
                // file.
                if ( mapping_.count(id) == 0 )
                {
                    // Check if the filepath is valid by opening it
                    // and checking it isuccessfully opened.
                    std::ifstream fs;
                    fs.open( file, std::ifstream::in);
                    if ( fs.is_open() )
                    {
                        // If the file successfully open then we can add
                        // the entry into the list of file to serve.
                        mapping_[std::string(id)]=std::string(file);
                        std::cout << "\t[OK]" << std::endl;
                    }
                    else
                    {
                        // If the entry is invali we print some error message
                        // and cancel this entry.
                        std::cout << "\t[INVALID FILE NAME]" << std::endl;
                    }
                }
                else
                {
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
    bool can_stream(const std::string& id)
    {
        if ( mapping_.count(id) != 0)
        {
            return true;
        }
        return false;
    }

    /*****************************************************************************
    * @desc return the filename corresponding to the given ID.
    *****************************************************************************/
    std::string& get_filename(const std::string& id)
    {
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
class Client_writer : public Thread
{
public:
    Client_writer(uint32_t clientid, int socket, Server& server) :
            m_clientid(clientid),
            socket_(socket),
            server_(server)
    {
        m_writer = new Data_writer_socket(socket_);
        m_reader = new Data_reader_socket(socket_),
        m_breader = new Data_reader_blocking( m_reader );
        udp_writer_ = NULL;
    }

    virtual ~Client_writer()
    {
        delete m_writer;
        delete m_reader;
        shutdown(socket_, SHUT_RDWR);
        std::cout << " ["<<  m_clientid << "]: Finalized " << std::endl;
    }

    /*****************************************************************************
    * @desc Kill the sub-thread and wait their termination before to leave the
    * function.
    *****************************************************************************/
    void kill()
    {
    }

    /*****************************************************************************
    * @desc Start streaming a file to the given client.
    *****************************************************************************/
    void send_file(Data_reader& reader, Data_writer& writer)
    {
        uint64_t toread;
        uint64_t read;
        uint64_t lasttime=0;
        int buffersize = 1234;
        int elemread;
        int towrite, prog;

        RTTimer timer;
        timer.start();
        uint64_t totalsent=0;

        char buffer [1234];
        uint64_t totalread=0;
        uint64_t totalsend=0;

        while ( isrunning() && !reader.eof()  )
        {
            elemread = reader.get_bytes( buffersize, buffer );
            towrite = elemread;
            totalread+=elemread;

            while( towrite > 0 ){
              prog = writer.put_bytes( towrite, &buffer[elemread-towrite] );
	      if (prog == 0)
		break;
              towrite -= prog;
              totalsend+=prog;
            }
	    if (towrite > 0)
	      break;
            totalsent += elemread;
            if ( timer.measured_time()-lasttime >= 1 ) {
              lasttime = timer.measured_time();
              std::cout << " ["<<  m_clientid << "]: streaming speed: " << toMB(totalsent) / timer.measured_time();
              std::cout << "MB/s" << std::endl;
            }
        }
        std::cout << " ["<<  m_clientid << "]: streaming speed: " << toMB(totalsent) / timer.measured_time();
        std::cout << "MB/s"<< std::endl;
        std::cout << " ["<<  m_clientid << "]: data streamed: "<< toMB(totalsent) << "MB" << std::endl;
    }

    /*****************************************************************************
    * @desc Call by the super-class Thread. Process the user-request and start
    * serving files if needed.
    *****************************************************************************/
    void do_execute()
    {
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
        if ( protocol == "udp")
        {
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
        if ( server_.can_stream(request) )
        {
            /// If so, tell it to the client.
            *m_writer << "OK";

            /// Create the data-reader/writer to the client and start streaming the
            /// file
            Data_reader_file filereader("file://"+server_.get_filename(request));
            if ( udp_writer_ )
                send_file( filereader, *udp_writer_);
            else
                send_file( filereader, *m_writer);
        }
        else
        {
            /// If the file is not valid, tell it ot client and print and info msg.
            *m_writer << "NOT-OK";
            std::cerr << " ["<<  m_clientid << "]: Warning: a user requested an invalid file: [" << request << "]" << std::endl;
        }
        std::cout << " ["<<  m_clientid << "]: Finalizing servicing thread." << std::endl;

        /// The file is streamed or a problem arose. Shutdown the socket
        sleep(5);
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
};

typedef Client_writer* Client_writer_ptr;

/*******************************************************************************
* @class Client_writer_allocator
* @desc This create a new client_writer
*       for each incoming client.
*******************************************************************************/
class Client_writer_allocator : public Connexion_thread_allocator< Client_writer >
{
    Server& server_;
    uint32_t m_counter;
    std::vector<Client_writer_ptr> vector_clients_;
    Mutex m_mutex;

    /// ThreadPool client_thread_pool;
public:
    Client_writer_allocator(Server& server) : server_(server)
    {
        m_counter= 0;
    }

    virtual ~Client_writer_allocator()
    {
        for (int i=0;i<vector_clients_.size();i++)
        {
            vector_clients_[i]->kill();
        }

        /// This is pretty ugly And MUST be fixed.
        for (int i=0;i<vector_clients_.size();i++)
        {
            while ( vector_clients_[i]->isrunning() );
            delete vector_clients_[i];
        }
    }

    /*****************************************************************************
    * @desc This function is called by the Client_thread_allocator object to
    * allocate a new Client_writer object for each incoming connexion. This object
    * also keep a track of active connexions.
    *****************************************************************************/
    Client_writer* allocate(int socket)
    {
        RAIIMutex rc(m_mutex);

        /// Create a new client_writer to handle the client request.
        Client_writer_ptr element = new Client_writer(m_counter++, socket, server_);

        /// This is buggy the thread pool will not like some object to be
        /// client_thread_pool.register_thread();

        /// Check if some clients became inactive.
        for (int i=0;i<vector_clients_.size();i++)
        {
            if ( !vector_clients_[i]->isrunning() )
            {
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

void print_usage(char* appname)
{
    std::cerr << "Syntax: "<< appname << "  [--if listeningip(default=any)] "
              << " [--port listeningport(default=2630)]" << std::endl;
}

/* Server for the SC07 demo */
int main(int argc, char** argv)
{
    try
    {
        signal(SIGPIPE, signal_handler);
        InterfaceIP* interface=Network::get_any_interface();
        int port=2630;
        bool arg_problem=false;
        for (int i=1;i<argc;i++)
        {
            if ( !strcmp(argv[i],"--if") )
            {
                if ( ++i < argc )
                {
                    interface= Network::get_interface_by_name(argv[i]);
                }
                else arg_problem= true;
            }
            else if ( !strcmp(argv[i],"--port") )
            {
                if ( ++i < argc )
                    port=atoi(argv[i]);
                else arg_problem= true;
            }
            else if ( !strcmp(argv[i], "--help") )
            {
                print_usage(argv[0]);
                exit(0);
            }
            else
            {
                arg_problem= true;
            }

            if (arg_problem)
            {
                if ( i < argc )
                    std::cerr << "Invalid argument: [" << argv[i] << "]" << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        }

        if (interface == NULL)
        {
            std::cerr << "Unable  to find a valid interface !" <<std::endl;
            std::vector<InterfaceIP*> intf;
            Network::get_interfaces(intf);
            std::cerr << "valid IPV4 interfaces are:" << std::endl;
            for (int i=0;i<intf.size();i++)
            {
                std::cerr << "\t" << intf[i]->name() << "(" << intf[i]->ip() << ")" << std::endl;
            }
            exit(2);
        }

        Connexion_listener_ptr listener = interface->create_listener( port );

        if ( listener == NULL )
        {
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
    }
    catch (Exception &e)
    {
        std::cerr << e << std::endl;
    }
}

