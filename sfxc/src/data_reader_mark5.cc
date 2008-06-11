/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "data_reader_mark5.h"
#include "utils.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <netinet/in.h> // sockaddr_in
#include <netdb.h>      // getservbyname()

Data_reader_mark5::Data_reader_mark5(const std::string& str) {
  int port;
  int idx;
  std::string protocol, tmp;

  idx = str.find("mark5://");
  if (  idx != 0 ) {
    std::cerr << "ERROR: this is not a valid mark5 string " << str << std::endl;
    exit(1);
  }
  tmp=str.substr(strlen("mark5://"));
  idx = tmp.find(":");

  if (  idx == (int)std::string::npos ) {
    std::cerr << "ERROR: this is not a valid mark5 string " << str << std::endl;
    exit(1);
  }
  protocol = tmp.substr(0, idx);
  tmp      = tmp.substr(idx+1);

  std::cout << "DEBUG: mark5 is using [" << protocol << "] and ports [" << tmp << "]" << std::endl;
  std::stringstream t(tmp);
  t >> port;
  initialize( protocol.c_str(), port );
}

Data_reader_mark5::Data_reader_mark5(const char *protocol, int port) {
  initialize(protocol, port);
}


void Data_reader_mark5::initialize(const char *protocol, int port) {
  int protocol_type, unconnected_sock;

  msglev=-1;
  _eof = false;

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
  if (setsockopt(unconnected_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on,
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

Data_reader_mark5::~Data_reader_mark5() {}

int Data_reader_mark5::do_get_bytes(size_t nBytes, char*out) {
  uint64_t size = 0, last;
  size = last = recv(sock, (void *) out, nBytes, 0);
  while (size < nBytes) {
    last = recv(sock, (void *) (out+size), nBytes-size, 0);
    if (last == 0) {
      _eof = true;
      return size;
    }
    size += last;
  }

  return nBytes;
}

bool Data_reader_mark5::eof() {
  SFXC_ASSERT_MSG(false,
                  "Data_reader_mark5::eof() not yet implemented");
  return false;
}


bool Data_reader_mark5::can_read() {
  DEBUG_MSG("Data_reader_file: can read not implemented");
  return true;
}
