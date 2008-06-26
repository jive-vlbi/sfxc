/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "single_data_writer_controller.h"
#include "sfxc_mpi.h"
#include "mpi_transfer.h"
#include "data_writer_file.h"
#include "data_writer_tcp.h"
#include "data_writer_socket.h"
#include "network.h"
#include "interface.h"

#include "utils.h"

Single_data_writer_controller::
Single_data_writer_controller(Node &node)
    : Controller(node) {
  int port = SFXC_PORT;
  while (!tcp_connection.open_port(port, /*max_connections*/1)) {
    port++;
  }
}

Single_data_writer_controller::
~Single_data_writer_controller() {
  buffer2writer.stop();
}

Single_data_writer_controller::Process_event_status
Single_data_writer_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
		case MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

			uint32_t info[4];
			std::vector<uint64_t> ip_ports;
			MPI_Transfer::recv_connect_writer_to_msg(info, ip_ports, status.MPI_SOURCE);

			//DEBUG_MSG("SINGLE DATA Connexion: " << info[0] << " ->" <<  info[2] );
			//DEBUG_MSG("SINGLE DATA  ip address:" <<  ip_ports.size() );

      CHECK_MPI( MPI_Ssend(&info, 4, MPI_UINT32,
													 info[2], MPI_TAG_ADD_TCP_READER_CONNECTED_FROM,
													 MPI_COMM_WORLD ) );

			//DEBUG_MSG("SINGLE DATA  connecting !:" );

			// Connect to the given host
			pConnexion cnx= NULL;
			for(unsigned int i=0;i<ip_ports.size() && cnx == NULL;i+=2){
					try{
						cnx = Network::connect_to( ip_ports[i], ip_ports[i+1] );
					}catch(Exception& e){}
			}

			if( cnx != NULL ){
				boost::shared_ptr<Data_writer>
				reader( new Data_writer_socket( cnx ) );
				set_data_writer(info[1], reader);
			}else{
				MTHROW("Unable to connect");
			}

			CHECK_MPI( MPI_Send(NULL, 0, MPI_UINT32,
													 status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED,
													 MPI_COMM_WORLD ) );

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }


  case MPI_TAG_ADD_DATA_WRITER_FILE2: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      SFXC_ASSERT(size > 0);
      char msg[size];
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      int stream_nr;
      memcpy(&stream_nr, msg, sizeof(int32_t));
      char *filename = msg+sizeof(int32_t);

      SFXC_ASSERT(status.MPI_SOURCE == status2.MPI_SOURCE);
      SFXC_ASSERT(status.MPI_TAG == status2.MPI_TAG);

      boost::shared_ptr<Data_writer> writer(new Data_writer_file(filename));
      set_data_writer(stream_nr, writer);

      MPI_Send(&stream_nr, 1, MPI_INT32,
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED,
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_TCP: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

      MPI_Status status2;

      /* - int32_t: stream number for the data writer
       * - int32_t: stream number for the data reader
       * - int32_t: rank of the data_reader
       */
      int32_t ranks[3];
      MPI_Recv(ranks, 3, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      Data_writer_tcp *data_writer = new Data_writer_tcp();

      if (tcp_connection.get_port() < 0) {
        tcp_connection.open_port(SFXC_PORT, MAX_TCP_CONNECTIONS);
      }
      SFXC_ASSERT(tcp_connection.get_port() > 0);

      std::vector<uint64_t>  ip_addresses;
      // Add number of the data stream:
      ip_addresses.push_back(ranks[2]);
      // Add the ip addresses
      tcp_connection.get_ip_addresses(ip_addresses);

      // Add port
      ip_addresses.push_back(tcp_connection.get_port());

      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_INT64,
               ranks[1], MPI_TAG_ADD_DATA_READER_TCP2, MPI_COMM_WORLD);

      data_writer->open_connection(tcp_connection);

      boost::shared_ptr<Data_writer> writer(data_writer);
      set_data_writer(ranks[0], writer);
      //add_data_writer(ranks[0], writer, ranks[2], ranks[1]);

      int32_t return_msg = 0;
      MPI_Recv(&return_msg, 1, MPI_INT32, ranks[1],
               MPI_TAG_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status2);

      MPI_Send(&ranks[0], 1, MPI_INT32,
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED,
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


Single_data_writer_controller::Queue_ptr
Single_data_writer_controller::queue() {
  SFXC_ASSERT(buffer2writer.get_queue() != Queue_ptr());
  return buffer2writer.get_queue();
}

void
Single_data_writer_controller::
set_queue(Queue_ptr queue) {
  buffer2writer.set_queue(queue);
  buffer2writer.try_start();
}

boost::shared_ptr<Data_writer>
Single_data_writer_controller::get_data_writer(int i) {
  return buffer2writer.get_data_writer();
}


void
Single_data_writer_controller::
set_data_writer(int streamnr, boost::shared_ptr<Data_writer> writer) {
  SFXC_ASSERT(streamnr == 0);
  SFXC_ASSERT(buffer2writer.get_data_writer() == NULL);
  buffer2writer.set_data_writer(writer);
  buffer2writer.try_start();

  node.hook_added_data_writer(streamnr);
}
