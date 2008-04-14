#include <cassert>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <ifaddrs.h>
#include "connexion_listener.h"

Connexion_listener::Connexion_listener(unsigned int serversocket, int port, InterfaceIP* interface) {
  verbose_ = true;
  m_serversocket = serversocket;
  m_interface = interface;
  m_port = port;

  if ( verbose_ )
    std::cout << "Connexion Listener created" << std::endl;
}

int Connexion_listener::wait_connexion() {
  int connectSocket;
  socklen_t clientAddressLength;
  struct sockaddr_in clientAddress;

  // Accept a connection with a client that is requesting one.  The
  // accept() call is a blocking call; i.e., this thread of
  // execution stops until a connection comes in.
  // connectSocket is a new socket that the system provides,
  // separate from listenSocket.  We *could* accept more
  // connections on listenSocket, before connectSocket is closed,
  // but this program doesn't do that.
  clientAddressLength = sizeof(clientAddress);

  if (verbose_)
    std::cout << "Waiting for connexion..." << this << std::endl;

  connectSocket = accept(m_serversocket,
                         (struct sockaddr *) &clientAddress,
                         &clientAddressLength);
  if (connectSocket < 0) {
    std::cout << "cannot accept connection ";
    return 0;
  }

  // Show the IP address of the client.
  // inet_ntoa() converts an IP address from binary form to the
  // standard "numbers and dots" notation.
  if (verbose_)
    std::cout << "  client want to connect from " << inet_ntoa(clientAddress.sin_addr);

  // Show the client's port number.
  // ntohs() converts a short int from network byte order (which is
  // Most Significant Byte first) to host byte order (which on x86,
  // for example, is Least Significant Byte first).
  if (verbose_)
    std::cout << ":" << ntohs(clientAddress.sin_port) << "\n";

  return connectSocket;
}

std::ostream& operator<<(std::ostream& out, Connexion_listener& intf) {
  out << " connexion listener on an ipv4 address: " << intf.address() << ":" << intf.port() ;
  return out;
}

std::ostream& operator<<(std::ostream& out, Connexion_listener* intf) {
  return out << *intf;
}
