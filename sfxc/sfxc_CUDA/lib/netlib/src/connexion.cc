
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <ifaddrs.h>

#include "connexion.h"
#include "exception_common.h"


int EndpointIP::get_port(){
    struct sockaddr_in addr;
    socklen_t addrlen=sizeof(addr);

    if( getsockname(m_socket, (struct sockaddr*)&addr, &addrlen) ){
        MTHROW("Unable to retreive the socket informations");
    }
    return ntohs(addr.sin_port);
}
