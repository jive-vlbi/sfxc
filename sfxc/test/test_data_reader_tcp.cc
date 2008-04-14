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

#include "sfxc_mpi.h"
#include "data_reader_file.h"
#include "data_reader_tcp.h"
#include "data_reader_buffer.h"
#include "data_reader2buffer.h"
#include "semaphore_buffer.h"
#include "data_writer_tcp.h"
#include "tcp_connection.h"

char *infile = "file://data/input.txt";
char *outfile = "file://output.txt";
#define BUFFSIZE 1000


int main(int argc, char *argv[]) {
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
    int size = sizeof(int32_t)+strlen(infile)+1;
    char message[size];
    int32_t stream_nr=0;
    memcpy(message,&stream_nr,sizeof(int32_t));
    memcpy(message+sizeof(int32_t), infile, strlen(infile)+1);
    MPI_Send(message, size, MPI_CHAR,
             1, MPI_TAG_ADD_DATA_READER, MPI_COMM_WORLD);

    TCP_Connection tcp_connection;
    int port = 1233;
    while (!tcp_connection.open_port(port, MAX_TCP_CONNECTIONS)) {
      port++;
    }
    assert(tcp_connection.get_port() > 0);
    Data_writer_tcp *data_writer = new Data_writer_tcp();

    std::vector<uint64_t>  ip_addresses;
    ip_addresses.push_back(0); // stream number
    tcp_connection.get_ip_addresses(ip_addresses);
    ip_addresses.push_back(tcp_connection.get_port());

    MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_INT64,
             1, MPI_TAG_ADD_DATA_READER_TCP2, MPI_COMM_WORLD);

    data_writer->open_connection(tcp_connection);

    // Transfer the data from the file:
    Data_reader *reader_file = new Data_reader_file(infile);
    int buffsize = 100, nread;
    char buff[buffsize];
    while ((nread = reader_file->get_bytes(buffsize, buff)) > 0) {
      char *data = buff;
      do {
        int read = data_writer->put_bytes(nread, buff);
        assert(read >= 0);
        nread -= read;
        data += read;
      } while (nread > 0);
    }

  } else { // Receiving node:
    MPI_Status status, status2;
    MPI_Probe(MPI_ANY_SOURCE, MPI_TAG_ADD_DATA_READER,
              MPI_COMM_WORLD, &status);
    int size;
    MPI_Get_elements(&status, MPI_CHAR, &size);
    assert(size > 0);
    char msg[size];
    MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);
    int stream_nr;
    memcpy(&stream_nr, msg, sizeof(int32_t));
    char *filename = msg+sizeof(int32_t);

    Data_reader *reader_file = new Data_reader_file(filename);


    MPI_Probe(MPI_ANY_SOURCE, MPI_TAG_ADD_DATA_READER_TCP2,
              MPI_COMM_WORLD, &status);
    MPI_Get_elements(&status, MPI_INT64, &size);
    assert(size > 0);
    uint64_t ip_addr[size];
    MPI_Recv(&ip_addr, size, MPI_INT64, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);

    assert(status.MPI_SOURCE == status2.MPI_SOURCE);
    assert(status.MPI_TAG == status2.MPI_TAG);

    boost::shared_ptr<Data_reader_tcp>
    reader_tcp(new Data_reader_tcp(ip_addr, size-1, ip_addr[size-1]));

    Data_reader2buffer< Buffer_element<char, 131072> > data_reader2buffer;
    data_reader2buffer.set_data_reader(reader_tcp);
    boost::shared_ptr< Semaphore_buffer<> > sem_buffer(new Semaphore_buffer<>(10));
    data_reader2buffer.set_buffer(sem_buffer);
    data_reader2buffer.try_start();
    Data_reader_buffer<> reader_buffer(data_reader2buffer.get_buffer());

    int buffsize = 10001, nread_file=1, nread_buffer;
    char buff_file[buffsize], buff_buffer[buffsize];
    while (nread_file > 0) {
      nread_file = reader_file->get_bytes(buffsize, buff_file);
      nread_buffer = 0;
      while (nread_buffer != nread_file) {
        int read = reader_buffer.get_bytes(nread_file-nread_buffer,
                                           &buff_buffer[nread_buffer]);
        assert(read >= 0);
        nread_buffer += read;
      }

      assert(nread_file == nread_buffer);
      assert(!memcmp(buff_file, buff_buffer, nread_file));
    }
  }

  return 0;
}
