#ifndef NETWORK_HH

#define NETWORK_HH



#include <vector>

#include <string>

#include "tcp_connection.h"
#include "common.h"

#include "interface.h"
#include "connexion.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

class Network {
public:
  static void get_interfaces(std::vector<InterfaceIP*>& intf);

  /// Return a list of interfaces, ordered by the given list of name
  static void get_interfaces_ordered_by_name(const Vector_string& prefferedif,
      std::vector<InterfaceIP*>& resif);

  static pInterfaceIP get_interface_by_name(const String& name);
  static pInterfaceIP get_interface_by_preffered(Vector_string& prefferedif);
  static pInterfaceIP get_first_interface();

  static pInterfaceIP get_any_interface();

  static pInterfaceIP scan_interfaces_for_dest(const std::string& ip);

  static pInterfaceIP scan_interfaces();

  static pConnexion connect_to(const std::string& ipaddress, unsigned short port, int type=SOCK_STREAM);
  static pConnexion connect_to(uint64_t ip, short port, int type=SOCK_STREAM);

  static EndpointIP* create_endpoint(unsigned short port=0);
protected:

private:

  static pInterfaceIP interface_any_;

};





#endif // NETWORK_HH

