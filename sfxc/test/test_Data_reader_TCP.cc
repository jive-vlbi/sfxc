/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
  Checks transferring data using a TCP connection.
  
  Program transfers a file using TCP, uses a buffer for asynchronous IO
  and compares it to the original file
*/

#include <assert.h>

char *infile = "data/input.txt";
char *outfile = "output.txt";
#define BUFFSIZE 1000

#include <sfxc_mpi.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Data_reader_buffer.h>
#include <Data_reader2buffer.h>
#include <Semaphore_buffer.h>
#include <Data_writer_tcp.h>
#include <TCP_Connection.h>

int main(int argc, char *argv[]) {
  if (argc >= 3) {
    infile  = argv[1];
    outfile = argv[2];
  }
  
  int numtasks, rank;
  { // MPI stuff
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
  }

  if (rank == 0) { // Sending node:
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(infile, strlen(infile)+1, MPI_CHAR, 
             1, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
    
    
    
    Data_writer_tcp *data_writer = new Data_writer_tcp(1233); 

    TCP_Connection tcp_connection;
    std::vector<UINT64>  ip_addresses;
    tcp_connection.get_ip_addresses(ip_addresses);
    ip_addresses.push_back(data_writer->get_port());
    
    MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UINT64, 
             1, MPI_TAG_ADD_DATA_READER_TCP, MPI_COMM_WORLD);

    data_writer->open_connection();
    
    // Transfer the data from the file:
    Data_reader *reader_file = new Data_reader_file(infile);
    int buffsize = 10000, nread;
    char buff[buffsize];
    while ((nread = reader_file->get_bytes(buffsize, buff)) > 0) {
      INT64 nwritten = data_writer->put_bytes(nread, buff);
      assert(nwritten == nread);
    }
    
  } else { // Receiving node:
    MPI_Status status, status2;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    int size;
    MPI_Get_elements(&status, MPI_CHAR, &size);
    assert(size > 0);
    char filename[size];
    MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);

    Data_reader *reader_file = new Data_reader_file(filename);
    
    
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Get_elements(&status, MPI_UINT64, &size);
    assert(size > 0);
    UINT64 ip_addr[size];
    MPI_Recv(&ip_addr, size, MPI_UINT64, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);
    
    assert(status.MPI_SOURCE == status2.MPI_SOURCE);
    assert(status.MPI_TAG == status2.MPI_TAG);

    Data_reader_tcp reader_tcp(ip_addr, size-1, ip_addr[size-1]);
    
    Data_reader2buffer< Buffer_element<char, 131072> > data_reader2buffer;
    data_reader2buffer.set_data_reader(&reader_tcp);
    Semaphore_buffer<> sem_buffer(10);
    data_reader2buffer.set_buffer(&sem_buffer);
    data_reader2buffer.try_start();
    Data_reader_buffer reader_buffer(data_reader2buffer.get_buffer());
    
    int buffsize = 10001, nread_file=1, nread_buffer;
    char buff_file[buffsize], buff_buffer[buffsize];
    while (nread_file > 0) {
      nread_file = reader_file->get_bytes(buffsize, buff_file);
      nread_buffer  = reader_buffer.get_bytes(nread_file, buff_buffer);
      
      assert(nread_file == nread_buffer);
      assert(!memcmp(buff_file, buff_buffer, nread_file));
    }
  }
  
  return 0;
}
