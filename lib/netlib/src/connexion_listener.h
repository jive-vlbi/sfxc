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
 *   - Description of a Connexion_listener object
 */
#ifndef CONNEXION_LISTENER_H
#define CONNEXION_LISTENER_H

#include "connexion.h"
#include "interface.h"

/**************************************
* @class Connexion_listener
* @desc A Connexion_listener waits for
* incoming connexion returning a socket
* identifier to the incoming connexion.
*
* This is still a transitional phase
* that make use of TCP_Connection object
***************************************/
class Connexion_listener
{
    int m_serversocket;
    int m_port;

    // The interface to which is bind the listener
    InterfaceIP* m_interface;

public:
    /****************************************
    * Create a new Connexion_listener object
    * serversocket is the socket on which
    * listen, port is the port to listen
    * and interface is the interface to usesss
    *****************************************/
    Connexion_listener(unsigned int serversocket, int port, pInterfaceIP interface);

    /************************************
    * start listening, block until a client
    * connects
    *************************************/
    int open_connexion();

    /************************************
    * return the port to which the client
    * have to connect.
    *************************************/
    int port()
    {
        return m_port;
    }

		/************************************
    * return the address/hostname associated
    * with this network interface in charge
    * of this communcation.
    *************************************/
    const String& address()
    {
        return m_interface->ip();
    }


    friend std::ostream& operator<<(std::ostream& out, Connexion_listener&);
    friend std::ostream& operator<<(std::ostream& out, Connexion_listener*);
};


typedef Connexion_listener* pConnexion_listener;

#endif // CONNEXION_LISTENER_H
