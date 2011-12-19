#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <iostream>

#include "network.h"
#include "exception_common.h"
#include "interface.h"
#include "connexion.h"

void
Network::get_interfaces_ordered_by_name(const Vector_string& names,
					std::vector<InterfaceIP*>& interfaces)
{
  std::vector<InterfaceIP*> unsorted_interfaces;
  std::vector<bool> selected;

  Network::get_interfaces(unsorted_interfaces);

  for (size_t i = 0; i < interfaces.size(); i++)
    selected.push_back(false);

  // Order the interfaces given by NAMES.
  for (size_t i = 0; i < names.size(); i++) {
    for (size_t j = 0; j < unsorted_interfaces.size(); j++) {
      if (names[i].find(unsorted_interfaces[j]->name()) != String::npos ||
	  names[i].find(unsorted_interfaces[j]->ip()) != String::npos ) {
        interfaces.push_back(unsorted_interfaces[j]);
        selected[j] = true;
      }
    }
  }

  // Add any remaining interfaces.
  for (size_t i = 0; i < unsorted_interfaces.size(); i++) {
    if (!selected[i])
      interfaces.push_back(unsorted_interfaces[i]);
  }
}

InterfaceIP*
Network::get_interface_by_name(const std::string& name)
{
  std::vector<InterfaceIP*> interfaces;

  Network::get_interfaces(interfaces);

  for (size_t i = 0; i < interfaces.size(); i++) {
    if (interfaces[i]->name().find(name) != String::npos)
      return interfaces[i];

    if (interfaces[i]->ip().find(name) != String::npos)
      return interfaces[i];
  }

  return NULL;
}

pInterfaceIP Network::interface_any_ = NULL;

InterfaceIP*
Network::get_any_interface()
{
  if (interface_any_ == NULL)
    interface_any_ = new InterfaceIP("any", "any");

  return interface_any_;
}

void
Network::get_interfaces(std::vector<InterfaceIP*>& interfaces)
{
  struct ifaddrs *ifa, *ifaddr;

  if (getifaddrs (&ifaddr) == -1)
    return;

  for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family != AF_INET)
      continue;
    if (ifa->ifa_flags & IFF_LOOPBACK)
      continue;

    char ip[NI_MAXHOST];
    if (getnameinfo (ifa->ifa_addr, sizeof(struct sockaddr_in),
		     ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0) {
      perror ("getnameinfo");
      continue;
    }

    std::string name = ifa->ifa_name;
    std::string address = ip;
    interfaces.push_back(new InterfaceIP(name, address));
  }

  freeifaddrs (ifa);
}

bool
Network::match_interface(in_addr_t ip)
{
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

pConnexion
Network::connect_to(const String& ipaddress, unsigned short port, int type)
{
  return get_any_interface()->connect_to(ipaddress, port, type);
}

pConnexion
Network::connect_to(uint64_t ip, short port, int type)
{
  return get_any_interface()->connect_to(ip, port, type);
}

EndpointIP*
Network::create_endpoint(unsigned short port)
{
  return get_any_interface()->create_endpoint(port);
}
