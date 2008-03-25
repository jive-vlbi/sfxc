#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <ifaddrs.h>

#include "common.h"
#include "exception_common.h"
#include "interface.h"
#include "connexion.h"
#include "connexion_listener.h"

std::ostream& operator<<(std::ostream& out, InterfaceIP& intf) {
  out << "name: " << intf.name() << " ipv4 address: " << intf.ip();
  return out;
}

std::ostream& operator<<(std::ostream& out, InterfaceIP* intf) {
  return out << *intf;
}


Connexion_listener* InterfaceIP::create_listener(const int portnum) {
  int serversocket = -1;
  int port= portnum;
  std::cout << "Hello:" << port << std::endl;

  while ( (serversocket = open_port( ip(), port)) == -1 ) {
    //std::cout << "Unable to bind on port:" << port << std::endl;
    port++;
    MASSERT(port < 65536 && "port is too high...something goes wrong");
  }

  std::cout << "Will create soon a connexion Listener:" << port << std::endl;
  return new Connexion_listener(serversocket, port, this);
}

int InterfaceIP::open_port(const String& interfacename, unsigned short int port) {
  int listenSocket;

  //  socklen_t clientAddressLength;
  struct sockaddr_in serverAddress;
  struct hostent *hostInfo;

  // Create socket for listening for client connection requests.

  listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    std::cerr << "cannot create listen socket" << std::endl;
    return -1;
  }

  // Bind listen socket to listen port.  First set various fields in
  // the serverAddress structure, then call bind().
  // htonl() and htons() convert long integers and short integers
  // (respectively) from host byte order (on x86 this is Least
  // Significant Byte first) to network byte order (Most Significant
  // Byte first).
  if ( interfacename != "any" ) {
    std::cout << "Not any" << std::endl;
    hostInfo = gethostbyname(interfacename.c_str());
    if (hostInfo == NULL) {
      std::cerr << "problem interpreting host: " << interfacename << "\n";
      return -1;
    }
    memcpy((char *) &serverAddress.sin_addr.s_addr,
           hostInfo->h_addr_list[0], hostInfo->h_length);
  } else {
    std::cout << "Is any" << std::endl;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
  }
  // Connect to server.  First we have to set some fields in the
  // serverAddress structure.  The system will assign me an arbitrary
  // local port that is not in use.
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);

  if (bind(listenSocket,
           (struct sockaddr *) &serverAddress,
           sizeof(serverAddress)) < 0) {
    std::cout << "cannot bind socket to " << interfacename << ":" << port << std::endl;
    return -1;
  }

  listen(listenSocket, 10);
  return listenSocket;
}

Connexion_listener* InterfaceIP::create_listener_autonum(const int portnum) {
  int serversocket = -1;
  int port= portnum;

  while ( (serversocket = open_port( ip(), port)) == -1 ) {
    //std::cout << "Unable to bind on port:" << port << std::endl;
    port++;
    MASSERT(port < 65536 && "port is too high...something goes wrong");
  }

  return new Connexion_listener(serversocket, port, this);
}

Connexion* InterfaceIP::connect_to(const std::string& ipaddress, const std::string& port) {
  uint16_t porto;
  std::istringstream s(port);
  s >> porto;
  return connect_to(ipaddress, porto);
}

Connexion* InterfaceIP::connect_to(const std::string& ipaddress, unsigned short port) {
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in localAddress;
  struct hostent *hostInfo;


  // Create a socket.  "AF_INET" means it will use the IPv4 protocol.
  // "SOCK_STREAM" means it will be a reliable connection (i.e., TCP;
  // for UDP use SOCK_DGRAM), and I'm not sure what the 0 for the last
  // parameter means, but it seems to work.
  socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (socketDescriptor < 0) {
    MTHROW("Unable to bind to a socket");
    return NULL;
  }


  // Get the info for the local host
  if ( name() == "any" ) {
    localAddress.sin_addr.s_addr = INADDR_ANY;
  } else {
    hostInfo = gethostbyname( ip().c_str() );
    if (hostInfo == NULL) {
      std::cout << "problem interpreting local host: " << ip() << "\n";
      return NULL;
    }
    memcpy((char *) &localAddress.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
  }

  //int window_size = 1024 * 1024; /* 128 kilobytes */
  //setsockopt(socketDescriptor, SOL_SOCKET, SO_SNDBUF, (char *) &window_size, sizeof(window_size));
  //setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVBUF, (char *) &window_size, sizeof(window_size));
  localAddress.sin_family = AF_INET;
  localAddress.sin_port = 0;

  // The socket is bounded to the local address
  if (bind(socketDescriptor, (struct sockaddr *) &localAddress, sizeof(localAddress)) == -1 ) {
    std::cout << "cannot bind socket to " << ip() << ":" << 0 << std::endl;
    close(socketDescriptor);
    MTHROW("Unable to bind to a socket");
  }


  // gethostbyname() takes a host name or ip address in "numbers and
  // dots" notation, and returns a pointer to a hostent structure,
  // which we'll need later.  It's not important for us what this
  // structure is actually composed of.
  hostInfo = gethostbyname(ipaddress.c_str());
  if (hostInfo == NULL) {
    std::cout << "problem interpreting host: " << ipaddress << "\n";
    MTHROW("Unable to find host:"+ipaddress);
    return NULL;
  }

  // Connect to server.  First we have to set some fields in the
  // serverAddress structure.  The system will assign me an arbitrary
  // local port that is not in use.
  serverAddress.sin_family = hostInfo->h_addrtype;
  memcpy((char *) &serverAddress.sin_addr.s_addr,
         hostInfo->h_addr_list[0], hostInfo->h_length);
  serverAddress.sin_port = htons(port);

  if (connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    std::cout << "cannot connect to port " << port << "\n";
    close(socketDescriptor);
    MTHROW("Unable to connect to "+ipaddress);
    return NULL;
  }

  return new Connexion(socketDescriptor);
}
