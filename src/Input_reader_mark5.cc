#include <Input_reader_mark5.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <netinet/in.h> // sockaddr_in
#include <netdb.h>      // getservbyname()

Input_reader_mark5::Input_reader_mark5(char *protocol, int port) 
  : buffer(), msglev(-1)
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

  //   /* If tcp, then we set keep-alive in case a firewall needs it */ 
  //   if (type == SOCK_STREAM) { 
  //     if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &tof, sizeof(int)) < 0) { 
  //       (void) fprintf(stderr, 
  //             "ERROR: setsockopt() SO_KEEPALIVE returned ", me); 
  //       perror("error"); 
  //       return(-5);  
  //     } 
  //   } 

  /* SO_RCVBUF sets or gets the maximum socket receive buffer in bytes.
   * The default value is set by the rmem_default sysctl, and the
   * maximum allowed value is set by the rmem_max sysctl. */ 
  // NGHK: Set later:
//   if (rcvbufs > 0) {
//     if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void *) &rcvbufs,
//                   sizeof(rcvbufs)) < 0) {
//       if (msglev < 2) { 
//         std::cerr << "WARNING:  setsockopt() SO_RCVBUF " << rcvbufs
//                   << " returned" << std::endl;
//         perror("error"); 
//       }
//     }
//   }
  
//   int optlen = sizeof(unsigned int);
//   (void) getsockopt(unconnected_sock, SOL_SOCKET, SO_RCVBUF, 
//                     (void *) &rcvbuf, &optlen); /* Socket receive-buffer size */ 
//   std::cerr << "DEBUG: rcvbufs is " << rcvbuf << "bytes" << std::endl; 
  
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

Input_reader_mark5::~Input_reader_mark5() {
}

INT64 Input_reader_mark5::move_forward(INT64 nBytes) {
  assert(nBytes >= 0);
  if (buffer.size() > nBytes) {
    assert(nBytes < buffer.capacity());
    buffer.erase(buffer.begin(), buffer.begin()+nBytes);
    return nBytes;
  } else {
    // Completely empty the buffer and forward the filepointer.
    INT64 size = buffer.size();
    buffer.clear();
    char *tmp_buff = new char[nBytes-size];
    size += recv(sock, (void *) tmp_buff, nBytes-size, 0);
    return size;
  }
}

INT64 Input_reader_mark5::get_bytes(INT64 nBytes, char*out) {
  if (nBytes > buffer.capacity()) {
    buffer.reserve(nBytes);
  }
  if (nBytes > buffer.size()) {
    // always completely fill the buffer
    INT64 nRead = buffer.capacity() - buffer.size();
    char *tmp_buff = new char[nRead];
    /* Read data from socket */ 
    INT64 size = recv(sock, (void *) tmp_buff, nRead, 0);

    std::vector<char>::iterator it = buffer.end();
    buffer.resize(buffer.size()+size);
    std::copy(tmp_buff, tmp_buff + size, it);

    delete[] tmp_buff;
  }

  std::copy(buffer.begin(), 
	    buffer.begin() + std::min(buffer.size(), (size_t)nBytes),
	    out);
  return std::min(buffer.size(), (size_t)nBytes);
}
