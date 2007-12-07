/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - netlib library
 * This file contains:
 *   - declaration of the Interface object.
 *   - declaration of the InterfaceIP object.
 */
#ifndef INTERFACE_H
#define INTERFACE_H

#include "common.h"
#include "tcp_connection.h"

class Connexion;
class Connexion_listener;

/************************************
* @class Interface
* @desc An Interface object is used
* to represent a network interface that
* can be used to socket based
* communcation.
*************************************/
class Interface
{
    String m_name;
    public:
        Interface(const std::string& name)
        {
                m_name = name;
        }

        const std::string& name(){return m_name;}
};


/************************************
* @class InterfaceIP
* @desc An InterfaceIP can be used to
* manipulate interface that are
* accessible through the are in the IP
* domain.
*************************************/
class InterfaceIP : public Interface
{
    std::string m_address;

    //TCP_Connection m_connection;
public:
    InterfaceIP(const std::string& name, const std::string& ip) :
            Interface(name)
        {
             m_address=ip;
        }

    const std::string& ip(){return m_address;}

    friend std::ostream& operator<<(std::ostream& out, InterfaceIP&);
    friend std::ostream& operator<<(std::ostream& out, InterfaceIP*);

    // return a valid connexion listener listening on this interface at the given port
    Connexion_listener* create_listener(const int port);

    // return a valid connexion listener objet, of the given port is used try to
    // bind to successive port ids.
    Connexion_listener* create_listener_autonum(const int portnum);

    // return a valid connexion to the esthablished endpoint
    Connexion* connect_to(const std::string& ipaddress, const std::string& port);
    Connexion* connect_to(const std::string& ipaddress, unsigned short port);

    private:
			int open_port(const String& interfacename, unsigned short int port);
};

typedef Interface* pInterface;
typedef InterfaceIP* pInterfaceIP;
#define noInterface NULL

#endif // INTERFACE_H
