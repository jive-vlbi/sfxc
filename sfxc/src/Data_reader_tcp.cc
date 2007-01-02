#include <Data_reader_tcp.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <netinet/in.h> // sockaddr_in
#include <netdb.h>      // getservbyname()

Data_reader_tcp::Data_reader_tcp(int port) 
  : buffer(), msglev(-1)
{
  int unconnected_sock;
  if ((unconnected_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) { 
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
    while ((k = bind(unconnected_sock, (struct sockaddr *) &socaddin,
                  sizeof(struct sockaddr_in))) < 0) { 
      std::cerr << "Warning: bind() returned " << k 
                << " for port " << socaddin.sin_port << std::endl; 
      socaddin.sin_port += 1;
      //exit(1);
    } 
  }

  if (msglev < 1) {
    std::cerr << "DEBUG: Socket "<< unconnected_sock << " bound to m5data " 
              << std::endl; 
  }

  {
    /* Listen for connections on this socket */
    int k;
    if ((k = listen(unconnected_sock, 3)) < 0) {
      std::cerr << "ERROR: listen() returned " << k << std::endl;
      exit(1);
    } 
  }

  if (msglev < 1) {
    std::cerr << "DEBUG: Listening for connections on socket "
              << unconnected_sock << std::endl; 
  }

  /* Accept a connection on this socket */ 
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
}

Data_reader_tcp::~Data_reader_tcp() {
}

UINT64 Data_reader_tcp::move_forward(UINT64 nBytes) {
  assert(nBytes >= 0);
  if (buffer.size() > nBytes) {
    assert(nBytes < buffer.capacity());
    buffer.erase(buffer.begin(), buffer.begin()+nBytes);
    return nBytes;
  } else {
    // Completely empty the buffer and forward the filepointer.
    UINT64 size = buffer.size();
    buffer.clear();
    char *tmp_buff = new char[nBytes-size];
    size += recv(sock, (void *) tmp_buff, nBytes-size, 0);
    return size;
  }
}

UINT64 Data_reader_tcp::get_bytes(UINT64 nBytes, char*out) {
  if (nBytes > buffer.capacity()) {
    buffer.reserve(nBytes);
  }
  if (nBytes > buffer.size()) {
    // always completely fill the buffer
    UINT64 nRead = buffer.capacity() - buffer.size();
    char *tmp_buff = new char[nRead];
    /* Read data from socket */ 
    UINT64 size = recv(sock, (void *) tmp_buff, nRead, 0);

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
