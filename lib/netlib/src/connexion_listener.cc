#include <cassert>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <ifaddrs.h>
#include "connexion_listener.h"

Connexion_listener::Connexion_listener(unsigned int serversocket, int port, InterfaceIP* interface)
{
    m_serversocket = serversocket;
    m_interface = interface;
    m_port = port;
}

int Connexion_listener::open_connexion()
{
		assert(false && "Check for deprecation");
    //return m_connection.open_connection(m_serversocket);
}

std::ostream& operator<<(std::ostream& out, Connexion_listener& intf)
{
    out << " connexion listener on an ipv4 address: " << intf.address() << ":" << intf.port() ;
    return out;
}

std::ostream& operator<<(std::ostream& out, Connexion_listener* intf)
{
    return out << *intf;
}
