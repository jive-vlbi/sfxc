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
    int listenSocket = connection.open_port(1233);
    assert(listenSocket > 0);
    int connectSocket = connection.open_connection(listenSocket);
    assert(connectSocket > 0);
    connectSocket = connection.open_connection(listenSocket);
    assert(connectSocket > 0);
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
    }
    {
      std::vector<UINT64> addr;
      connection.get_ip_addresses(addr);
      int connectSocket = -1;
  
      std::cout << std::hex;
      for (std::vector<UINT64>::iterator it = addr.begin(); 
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
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
