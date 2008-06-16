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
 *   - a Connexion object that wrap around a socket.
 */
#ifndef CONNEXION_H
#define CONNEXION_H

#include <sys/socket.h>

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

/****************************************
* @class Connexion
* @desc wrap around a socket descriptor
****************************************/
class Connexion {
public:
  /************************************
  * Create a Connexion object. The
  * objet is in a non-connected state.
  *************************************/
  Connexion() {
    m_socket = -1;
  }

  /************************************
  * Create a Connexion object using the
  * given socket. The object is in a
  * connected state.
  *************************************/
  Connexion(int socket) {
    m_socket = socket;
  }

  /***************************************
  * Return true if the connexion is in a
  * connected state.
  ***************************************/
  bool is_connected() {
    return m_socket >= 0;
  }

  /***************************************
  * Accessor to the underlying socket id
  ***************************************/
  int get_socket() {
    return m_socket;
  }
  int socket() {
    return m_socket;
  }

  // those two function have to be implemented
  //void connect_to();
  //void connect_to(pInterface);
protected:
  int m_socket;
};

typedef Connexion* pConnexion;

/****************************************
* @class Connexion
* @desc wrap around a socket descriptor
****************************************/
class EndpointIP {
public:
  /************************************
  * Create a Connexion object. The
  * objet is in a non-connected state.
  *************************************/
  EndpointIP() {
    m_socket = -1;
  }

  /************************************
  * Create a Connexion object using the
  * given socket. The object is in a
  * connected state.
  *************************************/
  EndpointIP(int socket) {
    m_socket = socket;
  }


  /***************************************
  * Accessor to the underlying socket id
  ***************************************/
  int get_socket() {
    return m_socket;
  }

  int socket() {
    return m_socket;
  }

  int get_port();

  // those two function have to be implemented
  //void connect_to();
  //void connect_to(pInterface);
protected:
  int m_socket;
};




#endif // CONNEXION_H
