#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <unistd.h>

#include <iostream>

#include "network.h"

#include "exception_common.h"

#include "interface.h"

#include "connexion.h"




void Network::get_interfaces_ordered_by_name(const Vector_string& ifname, std::vector<InterfaceIP*>& ifs) {
  /// We retreive the list of network interfaces
  std::vector<InterfaceIP*> interfaces;
  std::vector<bool> selected;

  /// Get the list of interfaces...
  Network::get_interfaces(interfaces);

  for (unsigned int i=0;i<interfaces.size();i++)
    selected.push_back(false);

  /// order the given interface
  for (unsigned int i=0;i<ifname.size();i++) {
    for (unsigned int j=0;j<interfaces.size();j++) {
      if ( ifname[i].find( interfaces[j]->name() ) != String::npos  ||
           ifname[i].find( interfaces[j]->ip() ) != String::npos ) {
        ifs.push_back( interfaces[j] );
        selected[j] = true;
      }
    }
  }

  /// add the remaining ones.
  for (unsigned int i=0;i<interfaces.size();i++) {
    if ( !selected[i] )
      ifs.push_back( interfaces[i] );
  }
}



InterfaceIP* Network::get_interface_by_name(const std::string& name) {

  std::vector<InterfaceIP*> interface;



  Network::get_interfaces(interface);

  for (unsigned int i=0;i<interface.size();i++) {

    if ( interface[i]->name().find( name ) != String::npos ) return interface[i];

    if ( interface[i]->ip().find( name ) != String::npos ) return interface[i];
  }


  return NULL;

}



InterfaceIP* Network::get_first_interface() {

  std::vector<InterfaceIP*> interface;

  Network::get_interfaces(interface);

  if ( interface.size() != 0 ) return interface[0];

  return NULL;

}



pInterfaceIP Network::interface_any_ = NULL;

InterfaceIP* Network::get_any_interface() {

  if ( interface_any_ == NULL )

    interface_any_ = new InterfaceIP("any", "any");



  return interface_any_;

}



void Network::get_interfaces(std::vector<InterfaceIP*>& vectorinterfaces) {

  struct ifaddrs *ifa = NULL;



  if (getifaddrs (&ifa) < 0) {

    std::cout << "error in getifaddrs" << std::endl;

    return;

  }



  for (; ifa; ifa = ifa->ifa_next) {

    char ip[ NI_MAXHOST ];

    socklen_t salen;


    if (ifa->ifa_flags & IFF_LOOPBACK)
      continue;


    //std::cout << "scanning network interface:" << ifa->ifa_name << std::endl;

    if (ifa->ifa_addr->sa_family == AF_INET) {

      // IP v4

      salen = sizeof (struct sockaddr_in);

      if (getnameinfo (ifa->ifa_addr, salen,

                       ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0) {

        perror ("getnameinfo");

        continue;

      }

      std::string name = ifa->ifa_name;

      std::string address = ip;

      vectorinterfaces.push_back( new InterfaceIP(name, address) );

    }

  }



  freeifaddrs (ifa);

}


bool Network::match_interface(in_addr_t ip) {
  struct ifaddrs *ifa, *ifaddr;
  in_addr_t addr, mask;

  if (getifaddrs(&ifaddr) == -1)
    return false;

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family != AF_INET)
      continue;
    if (ifa->ifa_flags & IFF_LOOPBACK)
      continue;

    addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
    mask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
    if ((ip & mask) == (addr & mask))
      return true;
  }

  return false;
}


InterfaceIP* Network::scan_interfaces() {

  // Scanning available network interfaces

  InterfaceIP *interface = NULL;

  interface = Network::get_interface_by_name("myri0");

  if ( !interface ) {

    interface = Network::get_interface_by_name("eth0");

    if ( !interface ) {

      interface =Network::get_first_interface();

      if ( !interface) MTHROW("Unable to find a suitable network interface");

    }

  }

  return interface;

}





InterfaceIP* Network::get_interface_by_preffered(Vector_string& prefferedif) {

  std::vector<InterfaceIP*> interface;



  Network::get_interfaces(interface);

  for (unsigned int j =0;j<prefferedif.size();j++ ) {

    String& name = prefferedif[j];

    for (unsigned int i=0;i<interface.size();i++) {

      if ( interface[i]->name().find( name ) != String::npos ) return interface[i];

    }

  }

  return NULL;

}

pConnexion Network::connect_to(const String& ipaddress, unsigned short port, int type) {
  return get_any_interface()->connect_to(ipaddress, port, type);
}


pConnexion Network::connect_to(uint64_t ip, short port, int type) {
  return get_any_interface()->connect_to(ip, port, type);
}

EndpointIP* Network::create_endpoint(unsigned short port) {
  return get_any_interface()->create_endpoint(port);
}
