/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "mpi_transfer.h"
#include "types.h"
#include "utils.h"
#include "exception_common.h"

#include <iostream>

MPI_Transfer::MPI_Transfer() {}


void MPI_Transfer::send_ip_address(std::vector<uint64_t>& param, const int rank) {
  /// The param argument should contains a list of IP-port addresses
  SFXC_ASSERT( param.size() > 0);
  SFXC_ASSERT( param.size() % 2 == 0);

  CHECK_MPI( MPI_Send(&param[0], param.size(), MPI_INT64,
											rank, MPI_TAG_CONNEXION_INFO,
											MPI_COMM_WORLD) );
}

void MPI_Transfer::receive_ip_address(std::vector<uint64_t>& param, const int rank) {
  MPI_Status status;
  int size;
	MPI_Probe(rank, MPI_TAG_CONNEXION_INFO, MPI_COMM_WORLD, &status);
  MPI_Get_elements(&status, MPI_INT64, &size);

  SFXC_ASSERT(size > 0);
  SFXC_ASSERT(size % 2 == 0);

  param.resize(size);

  CHECK_MPI( MPI_Recv(&param[0],
										  size,
                      MPI_INT64, rank,
                      MPI_TAG_CONNEXION_INFO, MPI_COMM_WORLD, &status) );
}


void
MPI_Transfer::
send_connect_to_msg(const uint32_t info[4], const std::vector<uint64_t>& params, const int rank) {

  int size = 4*sizeof(int32_t) + params.size()*sizeof(uint64_t);
  int position=0;
  char buffer[size];

  CHECK_MPI( MPI_Pack( const_cast<void*>( (void*)info ), 4, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD) );

  CHECK_MPI( MPI_Pack(  const_cast<void*>( (void*) &params[0] ), params.size(), MPI_INT64,
						buffer, size, &position, MPI_COMM_WORLD) );

  SFXC_ASSERT(position == size);

  CHECK_MPI(
					  MPI_Send(buffer, size,
						MPI_CHAR, rank,
						MPI_TAG_ADD_TCP_READER_CONNECTED_TO,
						MPI_COMM_WORLD)
						);
}

void
MPI_Transfer::
recv_connect_to_msg(uint32_t info[4], std::vector<uint64_t>& params, const int rank) {
	MPI_Status status;
  int size;

	CHECK_MPI( MPI_Probe(rank, MPI_TAG_ADD_TCP_READER_CONNECTED_TO, MPI_COMM_WORLD, &status) );
  CHECK_MPI( MPI_Get_elements(&status, MPI_CHAR, &size) );
	char buffer[size];

	int num_params = (size-( sizeof(uint32_t)*4 ))/sizeof(uint64_t);


	params.resize( num_params );

  CHECK_MPI ( MPI_Recv(buffer, size,
											 MPI_CHAR, rank,
											 MPI_TAG_ADD_TCP_READER_CONNECTED_TO,
											 MPI_COMM_WORLD, &status) );

  int position=0;

  CHECK_MPI( MPI_Unpack(buffer, size, &position, info,
												4, MPI_UINT32, MPI_COMM_WORLD) );

  CHECK_MPI( MPI_Unpack(buffer, size, &position, &params[0],
												num_params, MPI_INT64, MPI_COMM_WORLD) );

  SFXC_ASSERT(position == size);
}


void
MPI_Transfer::
send_connect_writer_to_msg(const uint32_t info[4], const std::vector<uint64_t>& params, const int rank) {

  int size = 4*sizeof(int32_t) + params.size()*sizeof(uint64_t);
  int position=0;
  char buffer[size];

  CHECK_MPI( MPI_Pack( const_cast<void*>( (void*)info ), 4, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD) );

  CHECK_MPI( MPI_Pack(  const_cast<void*>( (void*) &params[0] ), params.size(), MPI_INT64,
						buffer, size, &position, MPI_COMM_WORLD) );

  SFXC_ASSERT(position == size);

  CHECK_MPI(
					  MPI_Send(buffer, size,
						MPI_CHAR, rank,
						MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO,
						MPI_COMM_WORLD)
						);
}

void
MPI_Transfer::
recv_connect_writer_to_msg(uint32_t info[4], std::vector<uint64_t>& params, const int rank) {
	MPI_Status status;
  int size;

	CHECK_MPI( MPI_Probe(rank, MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO, MPI_COMM_WORLD, &status) );
  CHECK_MPI( MPI_Get_elements(&status, MPI_CHAR, &size) );
	char buffer[size];

	int num_params = (size-( sizeof(uint32_t)*4 ))/sizeof(uint64_t);


	params.resize( num_params );

  CHECK_MPI ( MPI_Recv(buffer, size,
											 MPI_CHAR, rank,
											 MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO,
											 MPI_COMM_WORLD, &status) );

  int position=0;

  CHECK_MPI( MPI_Unpack(buffer, size, &position, info,
												4, MPI_UINT32, MPI_COMM_WORLD) );

  CHECK_MPI( MPI_Unpack(buffer, size, &position, &params[0],
												num_params, MPI_INT64, MPI_COMM_WORLD) );

  SFXC_ASSERT(position == size);
}


void
MPI_Transfer::
send(Delay_table_akima &table, int sn, int rank) {
  uint32_t n_datapoints = table.times.size();
  int size = 2*sizeof(int32_t) + 2*n_datapoints*sizeof(double);
  int position=0;
  char buffer[size];

  // First integer is the station number
  MPI_Pack(&sn, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);

  // Arrays
  // Send the length for convenience
  MPI_Pack(&n_datapoints, 1, MPI_UINT32, buffer, size,
           &position, MPI_COMM_WORLD);
  MPI_Pack(&table.times[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.delays[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  SFXC_ASSERT(position == size);

  MPI_Send(buffer, position, MPI_PACKED, rank,
           MPI_TAG_DELAY_TABLE, MPI_COMM_WORLD);
}

void
MPI_Transfer::
receive(MPI_Status &status, Delay_table_akima &table, int &sn) {
  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];

  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int position = 0;

  // First, get the station number
  MPI_Unpack(buffer, size, &position, &sn, 1, MPI_INT32, MPI_COMM_WORLD);

  // Arrays
  // first the size of the array
  uint32_t n_datapoints;
  MPI_Unpack(buffer, size, &position, &n_datapoints,
             1, MPI_UINT32, MPI_COMM_WORLD);
  table.times.resize(n_datapoints);
  table.delays.resize(n_datapoints);
  MPI_Unpack(buffer, size, &position, &table.times[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.times.size() == n_datapoints);
  MPI_Unpack(buffer, size, &position, &table.delays[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.delays.size() == n_datapoints);

  SFXC_ASSERT(position == size);

  table.begin_scan  = 0;
  table.end_scan    = 0;
  table.acc         = NULL;
  table.splineakima = NULL;
}

void
MPI_Transfer::
send(Uvw_model &table, int sn, int rank) {
  uint32_t n_datapoints = table.times.size();
  int size = 2*sizeof(int32_t) + 4*n_datapoints*sizeof(double);
  int position=0;
  char buffer[size];

  // First integer is the station number
  MPI_Pack(&sn, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);

  // Arrays
  // Send the length for convenience
  MPI_Pack(&n_datapoints, 1, MPI_UINT32, buffer, size,
           &position, MPI_COMM_WORLD);
  MPI_Pack(&table.times[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.u[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.v[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.w[0], n_datapoints, MPI_DOUBLE,
           buffer, size, &position, MPI_COMM_WORLD);
  SFXC_ASSERT(position == size);

  MPI_Send(buffer, position, MPI_PACKED, rank,
           MPI_TAG_UVW_TABLE, MPI_COMM_WORLD);
}

void
MPI_Transfer::
receive(MPI_Status &status, Uvw_model &table, int &sn) {
  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];

  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int position = 0;

  // First, get the station number
  MPI_Unpack(buffer, size, &position, &sn, 1, MPI_INT32, MPI_COMM_WORLD);

  // Arrays
  // first the size of the array
  uint32_t n_datapoints;
  MPI_Unpack(buffer, size, &position, &n_datapoints,
             1, MPI_UINT32, MPI_COMM_WORLD);
  table.times.resize(n_datapoints);
  table.u.resize(n_datapoints);
  table.v.resize(n_datapoints);
  table.w.resize(n_datapoints);

  MPI_Unpack(buffer, size, &position, &table.times[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.times.size() == n_datapoints);
  MPI_Unpack(buffer, size, &position, &table.u[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.u.size() == n_datapoints);
  MPI_Unpack(buffer, size, &position, &table.v[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.v.size() == n_datapoints);
  MPI_Unpack(buffer, size, &position, &table.w[0],
             n_datapoints, MPI_DOUBLE, MPI_COMM_WORLD);
  SFXC_ASSERT(table.w.size() == n_datapoints);

  SFXC_ASSERT(position == size);

  table.end_scan      = 0;
  table.acc_u         = NULL;
  table.acc_v         = NULL;
  table.acc_w         = NULL;
  table.splineakima_u = NULL;
  table.splineakima_v = NULL;
  table.splineakima_w = NULL;}


void
MPI_Transfer::send(Input_node_parameters &input_node_param, int rank) {
  int size = 0;
  size = 5*sizeof(int32_t)+2*sizeof(int16_t);
  for (Input_node_parameters::Channel_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++) {
    size +=
      sizeof(int32_t) *
      (4 +
       channel->sign_tracks.size() +
       channel->magn_tracks.size());
  }

  int position = 0;
  int32_t length;
  char message_buffer[size];

  MPI_Pack(&input_node_param.track_bit_rate, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.number_channels, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.integr_time, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.data_modulation, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.start_year, 1, MPI_INT16,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.start_day, 1, MPI_INT16,
           message_buffer, size, &position, MPI_COMM_WORLD);


  length = (int32_t)input_node_param.channels.size();
  MPI_Pack(&length, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  for (Input_node_parameters::Channel_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++) {
    // Sign
    MPI_Pack(&channel->sign_headstack, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    length = (int32_t)channel->sign_tracks.size();
    MPI_Pack(&length, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&channel->sign_tracks[0], length, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);

    // Magn
    MPI_Pack(&channel->magn_headstack, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    length = (int32_t)channel->magn_tracks.size();
    MPI_Pack(&length, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&channel->magn_tracks[0], length, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
  }
  SFXC_ASSERT(position == size);
  MPI_Send(message_buffer, position, MPI_PACKED, rank,
           MPI_TAG_TRACK_PARAMETERS, MPI_COMM_WORLD);
}

void
MPI_Transfer::receive(MPI_Status &status, Input_node_parameters &input_node_param) {
  input_node_param.channels.clear();

  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int32_t length;
  int position = 0;

  MPI_Unpack(buffer, size, &position,
             &input_node_param.track_bit_rate, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.number_channels, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.integr_time, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.data_modulation, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.start_year, 1, MPI_INT16,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.start_day, 1, MPI_INT16,
             MPI_COMM_WORLD);
  int32_t n_channels;
  MPI_Unpack(buffer, size, &position,
             &n_channels, 1, MPI_INT32,
             MPI_COMM_WORLD);
  while (n_channels > 0) {
    Input_node_parameters::Channel_parameters channel_param;
    // Sign
    MPI_Unpack(buffer, size, &position,
               &channel_param.sign_headstack, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &length, 1, MPI_INT32,
               MPI_COMM_WORLD);
    int tracks[length];
    MPI_Unpack(buffer, size, &position,
               &tracks, length, MPI_INT32,
               MPI_COMM_WORLD);
    for (int i=0; i<length; i++) {
      channel_param.sign_tracks.push_back(tracks[i]);
    }


    // Magn
    MPI_Unpack(buffer, size, &position,
               &channel_param.magn_headstack, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &length, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &tracks, length, MPI_INT32,
               MPI_COMM_WORLD);
    for (int i=0; i<length; i++) {
      channel_param.magn_tracks.push_back(tracks[i]);
    }

    input_node_param.channels.push_back(channel_param);

    n_channels--;
  }
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::send(Correlation_parameters &corr_param, int rank) {
  int size = 0;
  size =
    12*sizeof(int32_t) + sizeof(int64_t) + 3*sizeof(char) +
    corr_param.station_streams.size()*4*sizeof(int32_t);
  int position = 0;
  char message_buffer[size];

  MPI_Pack(&corr_param.start_time, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.stop_time, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.integration_time, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.number_channels, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.integration_nr, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.slice_nr, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.slice_offset, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&corr_param.sample_rate, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.bits_per_sample, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&corr_param.channel_freq, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.bandwidth, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.sideband, 1, MPI_CHAR,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.polarisation, 1, MPI_CHAR,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.channel_nr, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);

  char cross_polarize;
  if (corr_param.cross_polarize) {
    cross_polarize='y';
  } else {
    cross_polarize='n';
  }
  MPI_Pack(&cross_polarize, 1, MPI_CHAR,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.reference_station, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);

  for (Correlation_parameters::Station_iterator station =
         corr_param.station_streams.begin();
       station != corr_param.station_streams.end(); station++) {
    MPI_Pack(&station->station_number, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->station_stream, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->start_time, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->stop_time, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
  }
  SFXC_ASSERT(position == size);
  MPI_Send(message_buffer, position, MPI_PACKED, rank,
           MPI_TAG_CORR_PARAMETERS, MPI_COMM_WORLD);
}

void
MPI_Transfer::receive(MPI_Status &status, Correlation_parameters &corr_param) {
  corr_param.station_streams.clear();

  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  int position = 0;
  MPI_Unpack(buffer, size, &position,
             &corr_param.start_time, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.stop_time, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.integration_time, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.number_channels, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.integration_nr, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.slice_nr, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.slice_offset, 1, MPI_INT32,
             MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position,
             &corr_param.sample_rate, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.bits_per_sample, 1, MPI_INT32,
             MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position,
             &corr_param.channel_freq, 1, MPI_INT64,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.bandwidth, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.sideband, 1, MPI_CHAR,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.polarisation, 1, MPI_CHAR,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.channel_nr, 1, MPI_INT32,
             MPI_COMM_WORLD);
  char cross_polarize;
  MPI_Unpack(buffer, size, &position,
             &cross_polarize, 1, MPI_CHAR,
             MPI_COMM_WORLD);
  if (cross_polarize=='y') {
    corr_param.cross_polarize = true;
  } else {
    SFXC_ASSERT(cross_polarize=='n');
    corr_param.cross_polarize = false;
  }
  MPI_Unpack(buffer, size, &position,
             &corr_param.reference_station, 1, MPI_INT32,
             MPI_COMM_WORLD);

  while (position < size) {

    Correlation_parameters::Station_parameters station_param;
    MPI_Unpack(buffer, size, &position,
               &station_param.station_number, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.station_stream, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.start_time, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.stop_time, 1, MPI_INT32,
               MPI_COMM_WORLD);
    corr_param.station_streams.push_back(station_param);
  }

  SFXC_ASSERT(position == size);
}
