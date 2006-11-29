#include <Input_writer_mark5.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <netinet/in.h> // sockaddr_in
#include <netdb.h>      // getservbyname()

Input_writer_mark5::Input_writer_mark5(Input_reader& reader,
                                       char *protocol, int port) 
  : reader(reader) buffer(), msglev(-1)
{
  int protocol_type, unconnected_sock;
  // Check protocol
  if (strcasecmp(protocol, "tcp") == 0) {
    protocol_type = SOCK_STREAM; 
  } else if (strcasecmp(protocol, "udp") == 0) {
    protocol_type = SOCK_DGRAM; /* Yes */
  } else { 
    std::cerr << "ERROR: Unknown protocol: " << protocol << std::endl;
    exit(1);
  } 

  if ((unconnected_sock = socket(PF_INET, protocol_type, 0)) < 0) { 
    std::cerr << "ERROR: socket() returned " << unconnected_sock << std::endl;
    exit(1);
  }

  /* Get service number for socket service */
  /* Allow reuse of the local socket address in bind() */
  bool on = true;
  if(setsockopt(unconnected_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on,
                sizeof(int)) < 0) { 
    if (msglev < 2) {
      std::cerr << "WARNING: setsockopt() SO_REUSEADDR returned " << std::endl;
    } 
  }

  struct sockaddr_in socaddin;  /* For connect() socket info */ 

  socaddin.sin_port = port;

  if (msglev < 1) {
    std::cerr << "DEBUG: port is " << socaddin.sin_port << std::endl;
  }

  socaddin.sin_family = PF_INET; /* To agree with socket() above */
  socaddin.sin_addr.s_addr = INADDR_ANY; /* From any network address */ 

  {
    /* Bind this socket to service */
    int k;
    if ((k = bind(unconnected_sock, (struct sockaddr *) &socaddin,
                  sizeof(struct sockaddr_in))) < 0) { 
      std::cerr << "ERROR: bind() returned " << k << std::endl; 
      exit(1);
    } 
  }

  if (msglev < 1) {
    std::cerr << "DEBUG: Socket "<< unconnected_sock << " bound to m5data " 
              <<  protocol << std::endl; 
  }

  {
    /* Listen for connections on this socket */
    int k;
    if (protocol_type == SOCK_STREAM && (k = listen(unconnected_sock, 3)) < 0) {
      std::cerr << "ERROR: listen() returned " << k << std::endl;
      exit(1);
    } 
  }

  if (msglev < 1) {
    std::cerr << "DEBUG: Listening for connections on socket "
              << unconnected_sock << std::endl; 
  }

  /* Accept a connection on this socket */ 
  if (protocol_type == SOCK_STREAM) { /* tcp */ 
    socklen_t k = sizeof(struct sockaddr_in); 
    if (msglev < 1)  
      std::cerr << "DEBUG: Waiting on accept()" << std::endl; 
    /* (We usually hang here waiting for the Mark-5 machine to connect) */ 
    struct sockaddr_in socadd; /* For accept() socket info */
    if ((sock=accept(unconnected_sock, (struct sockaddr *) &socadd, &k))<0) { 
      std::cerr << "ERROR: accept() returned " << sock << std::endl;
      exit(1);
    } 
    
    /* Here we have an accept on the data socket and a new socket */
    if (msglev < 1) 
      std::cerr << "DEBUG: Got accept() on sock " 
                << unconnected_sock << " sock " <<  sock << std::endl; 
    close(unconnected_sock);
  } else { /* udp */ 
    sock = unconnected_sock; 
    if (msglev < 1) {
      std::cerr << "DEBUG: Ready to read " << protocol
                << "data on socket " << sock<< std::endl;
    }
  } 
}

Input_writer_mark5::~Input_writer_mark5() {
}
