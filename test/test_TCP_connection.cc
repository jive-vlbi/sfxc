/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "sfxc_mpi.h"
#include "TCP_Connection.h"

#include <arpa/inet.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

#include <iostream>
#include <assert.h>

int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
    return 1;
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  assert(numtasks==2);

  TCP_Connection connection;  
  if (rank==0) {
    char out_buff[100], in_buff[100];

    int port = 1233;
    int listenSocket=0;
    while (listenSocket <= 0) {
      listenSocket = connection.open_port(port);
      port ++;
    }
    std::cout << "listenSocket: " << listenSocket << " " << port << std::endl;
    assert(listenSocket > 0);
    int connectSocket = connection.open_connection(listenSocket);

    assert(connectSocket > 0);

    for (int i=0; i<100; i++) out_buff[i] = (char)i;
    send(connectSocket, out_buff, 100, 0);
    recv(connectSocket, in_buff, 100, 0);
    for (int i=0; i<100; i++) {
      assert(out_buff[i] == in_buff[i]);
      in_buff[i] = ' ';
    }
    shutdown(connectSocket, 2);
    
    connectSocket = connection.open_connection(listenSocket);

    assert(connectSocket > 0);

    send(connectSocket, out_buff, 100, 0);
    recv(connectSocket, in_buff, 100, 0);
    for (int i=0; i<100; i++) {
      assert(out_buff[i] == in_buff[i]);
    }
    shutdown(connectSocket, 2);
  } else {
    {
      std::vector<std::string> addr;
      connection.get_ip_addresses(addr);
      int connectSocket = -1;
  
      for (std::vector<std::string>::iterator it = addr.begin(); 
           connectSocket <= 0;) {
        if (*it != "127.0.0.1") {
          //std::cout << *it << std::endl;
          connectSocket = connection.do_connect(it->c_str(),1233);
        }
  
        it ++;
        if (it == addr.end()) it = addr.begin();
      }
      assert(connectSocket > 0);

      char buff[100]; 
      recv(connectSocket, buff, 100, 0);
      send(connectSocket, buff, 100, 0);
      
      shutdown(connectSocket, 2);
    }
    {
      std::vector<uint64_t> addr;
      connection.get_ip_addresses(addr);
      int connectSocket = -1;
  
      std::cout << std::hex;
      for (std::vector<uint64_t>::iterator it = addr.begin(); 
           connectSocket <= 0;) {
        struct in_addr addr1;
        addr1.s_addr = *it;
        if (connection.is_localhost(*it)) {
          connectSocket = connection.do_connect(*it,1233);
        }
  
        it ++;
        if (it == addr.end()) it = addr.begin();
      } 
      assert(connectSocket > 0);

      char buff[100]; 
      recv(connectSocket, buff, 100, 0);
      send(connectSocket, buff, 100, 0);

      shutdown(connectSocket, 2);
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
