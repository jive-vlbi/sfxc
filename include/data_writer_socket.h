/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 */
#ifndef DATAWRITER_SOCKET_HH
#define DATAWRITER_SOCKET_HH

#include <cassert>
#include "network.h"
#include "data_writer.h"

class DataWriter_socket : public Data_writer
{
    public:
        DataWriter_socket(int socket);
        DataWriter_socket(Connexion* connexion);
        virtual ~DataWriter_socket();

        //unsigned int get_port();
        void flush();
        void close(){ assert(false && "not implemented"); }

    protected:
        size_t do_put_bytes(size_t nBytes, char const*buff);
        int m_socket;
};

#endif // DATAWRITER_SOCKET_HH
