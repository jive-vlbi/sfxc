#ifndef NETWORK_HH
#define NETWORK_HH

#include <vector>
#include <string>
#include "tcp_connection.h"
#include "common.h"

#include "interface.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

class Network
{
    public:
        static void get_interfaces(std::vector<InterfaceIP*>& intf);
        static pInterfaceIP get_interface_by_name(const String& name);
        static pInterfaceIP get_interface_by_preffered(Vector_string& prefferedif);
        static pInterfaceIP get_first_interface();

        static pInterfaceIP scan_interfaces_for_dest(const std::string& ip);
        static pInterfaceIP scan_interfaces();
    protected:
    private:
};


#endif // NETWORK_HH
