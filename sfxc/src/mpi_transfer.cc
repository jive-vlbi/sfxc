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

void MPI_Transfer::send_ip_address(std::vector<uint64_t>& params, const int rank) {
  char hostname[HOST_NAME_MAX + 1];
  if (gethostname(hostname, sizeof(hostname)) == -1)
    hostname[0] = 0;

  uint32_t len = strlen(hostname);
  size_t size = sizeof(uint32_t) + len + params.size() * sizeof(uint64_t);
  char buffer[size];

  int position = 0;
  CHECK_MPI(MPI_Pack(&len, 1, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(hostname, len, MPI_CHAR, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(&params[0], params.size(), MPI_INT64, buffer, size, &position, MPI_COMM_WORLD));
  SFXC_ASSERT(position == size);

  CHECK_MPI(MPI_Send(buffer, size, MPI_CHAR, rank, MPI_TAG_CONNEXION_INFO, MPI_COMM_WORLD));
}

void MPI_Transfer::receive_ip_address(std::vector<uint64_t>& params, std::string& hostname, const int rank) {
  MPI_Status status;
  int size;

  MPI_Probe(rank, MPI_TAG_CONNEXION_INFO, MPI_COMM_WORLD, &status);
  MPI_Get_elements(&status, MPI_CHAR, &size);

  char buffer[size];
  CHECK_MPI(MPI_Recv(buffer, size, MPI_CHAR, rank, MPI_TAG_CONNEXION_INFO, MPI_COMM_WORLD, &status));

  int position = 0;
  uint32_t len;
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &len, 1, MPI_UINT32, MPI_COMM_WORLD));
  char str[len + 1];
  CHECK_MPI(MPI_Unpack(buffer, size, &position, str, len, MPI_CHAR, MPI_COMM_WORLD));
  str[len] = 0;
  hostname = str;
  int num_params = (size - position) / sizeof(uint64_t);
  params.resize(num_params);
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &params[0], num_params, MPI_INT64, MPI_COMM_WORLD));
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::
send_connect_to_msg(const uint32_t info[4], const std::vector<uint64_t>& params, const std::string& hostname, const int rank) {
  int size = sizeof(uint32_t) + hostname.size() + 4 * sizeof(int32_t) + params.size() * sizeof(uint64_t);
  int position = 0;
  char buffer[size];
  uint32_t len;

  len = hostname.size();
  CHECK_MPI(MPI_Pack(&len, 1, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void *)hostname.c_str()), len, MPI_CHAR, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void*)info), 4, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void*) &params[0]), params.size(), MPI_INT64,
		     buffer, size, &position, MPI_COMM_WORLD) );
  SFXC_ASSERT(position == size);

  CHECK_MPI(MPI_Send(buffer, size, MPI_CHAR, rank,
		     MPI_TAG_ADD_TCP_READER_CONNECTED_TO, MPI_COMM_WORLD));
}

void
MPI_Transfer::
recv_connect_to_msg(uint32_t info[4], std::vector<uint64_t>& params, std::string& hostname, const int rank) {
  MPI_Status status;
  int size;

  CHECK_MPI(MPI_Probe(rank, MPI_TAG_ADD_TCP_READER_CONNECTED_TO, MPI_COMM_WORLD, &status));
  CHECK_MPI(MPI_Get_elements(&status, MPI_CHAR, &size));

  char buffer[size];
  CHECK_MPI(MPI_Recv(buffer, size, MPI_CHAR, rank,
		     MPI_TAG_ADD_TCP_READER_CONNECTED_TO, MPI_COMM_WORLD, &status));

  int position = 0;
  uint32_t len;
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &len, 1, MPI_UINT32, MPI_COMM_WORLD));
  char str[len + 1];
  CHECK_MPI(MPI_Unpack(buffer, size, &position, str, len, MPI_CHAR, MPI_COMM_WORLD));
  str[len] = 0;
  hostname = str;
  CHECK_MPI(MPI_Unpack(buffer, size, &position, info,
		       4, MPI_UINT32, MPI_COMM_WORLD));
  int num_params = (size - (sizeof(uint32_t) + len + 4 * sizeof(uint32_t))) / sizeof(uint64_t);
  params.resize(num_params);
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &params[0],
		       num_params, MPI_INT64, MPI_COMM_WORLD));
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::
send_connect_writer_to_msg(const uint32_t info[4], const std::vector<uint64_t>& params, const std::string& hostname, const int rank) {
  int size = sizeof(uint32_t) + hostname.size() + 4 * sizeof(int32_t) + params.size() * sizeof(uint64_t);
  int position = 0;
  char buffer[size];
  uint32_t len;

  len = hostname.size();
  CHECK_MPI(MPI_Pack(&len, 1, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void *)hostname.c_str()), len, MPI_CHAR, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void*)info), 4, MPI_UINT32, buffer, size, &position, MPI_COMM_WORLD));
  CHECK_MPI(MPI_Pack(const_cast<void*>((void*) &params[0]), params.size(), MPI_INT64,
		     buffer, size, &position, MPI_COMM_WORLD) );
  SFXC_ASSERT(position == size);

  CHECK_MPI(MPI_Send(buffer, size, MPI_CHAR, rank,
		     MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO, MPI_COMM_WORLD));
}

void
MPI_Transfer::
recv_connect_writer_to_msg(uint32_t info[4], std::vector<uint64_t>& params, std::string& hostname, const int rank) {
  MPI_Status status;
  int size;

  CHECK_MPI(MPI_Probe(rank, MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO, MPI_COMM_WORLD, &status) );
  CHECK_MPI(MPI_Get_elements(&status, MPI_CHAR, &size));

  char buffer[size];
  CHECK_MPI(MPI_Recv(buffer, size, MPI_CHAR, rank,
		     MPI_TAG_ADD_TCP_WRITER_CONNECTED_TO, MPI_COMM_WORLD, &status));

  int position = 0;
  uint32_t len;
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &len, 1, MPI_UINT32, MPI_COMM_WORLD));
  char str[len + 1];
  CHECK_MPI(MPI_Unpack(buffer, size, &position, str, len, MPI_CHAR, MPI_COMM_WORLD));
  str[len] = 0;
  hostname = str;
  CHECK_MPI(MPI_Unpack(buffer, size, &position, info,
		       4, MPI_UINT32, MPI_COMM_WORLD));
  int num_params = (size - (sizeof(uint32_t) + len + 4 * sizeof(uint32_t))) / sizeof(uint64_t);
  params.resize(num_params);
  CHECK_MPI(MPI_Unpack(buffer, size, &position, &params[0],
		       num_params, MPI_INT64, MPI_COMM_WORLD));
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::
pack(std::vector<char> &buffer, Delay_table_akima &table, int sn[2]) {
  int32_t n_sources = table.sources.size();
  int32_t n_scans = table.scans.size();
  int32_t n_times = table.times.size();
  int32_t n_delays = table.delays.size();
  int32_t n_clocks = table.clock_offsets.size();
  int32_t size = 7 * sizeof(int32_t) + n_sources * 81 * sizeof(char) + 
                 n_scans * (2 * sizeof(int64_t) + 5 * sizeof(int32_t)) +
                 (n_times + 3*n_delays) * sizeof(double) +
                 n_clocks * (2 * sizeof(int64_t) + 2 * sizeof(double));

  buffer.resize(size);
  int position=0;

  // First integer is the station number
  MPI_Pack(sn, 2, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);

  // all sources
  MPI_Pack(&n_sources, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  for(int i = 0; i < n_sources; i++){
    char source[81];
    strncpy(source, table.sources[i].c_str(), 80);
    source[80] = 0;
    MPI_Pack(source, 81, MPI_CHAR, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  // all scans
  MPI_Pack(&n_scans, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  for(int i = 0; i < n_scans; i++){
    Delay_table_akima::Scan &scan = table.scans[i];
    int64_t ticks = scan.begin.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
    ticks = scan.end.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.source, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.times, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.delays, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.phases, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.amplitudes, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  // all times
  MPI_Pack(&n_times, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.times[0], n_times, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  // all delays
  MPI_Pack(&n_delays, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.delays[0], n_delays, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.phases[0], n_delays, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.amplitudes[0], n_delays, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  // all clocks
  MPI_Pack(&n_clocks, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  for (int i = 0; i < n_clocks; i++) {
    int64_t ticks = table.clock_starts[i].get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  MPI_Pack(&table.clock_offsets[0], n_clocks, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  for (int i = 0; i < n_clocks; i++) {
    int64_t ticks = table.clock_epochs[i].get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  MPI_Pack(&table.clock_rates[0], n_clocks, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);

  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::
send(Delay_table_akima &table, int station_nr, int rank) {
  int sn[2] = {station_nr, -1};
  std::vector<char> buffer;
  pack(buffer, table, sn);
  int size = buffer.size();
  MPI_Send(&buffer[0], size, MPI_PACKED, rank, MPI_TAG_DELAY_TABLE, MPI_COMM_WORLD);
}

void
MPI_Transfer::
bcast_corr_nodes(Delay_table_akima &table, int sn[2]){
  std::vector<char> buffer;
  pack(buffer, table, sn);
  int n_ranks, n_corr_nodes;
  MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
  MPI_Comm_size(MPI_COMM_CORR_NODES, &n_corr_nodes);
  // NB:: MPI_COMM_CORR_NODES includes the management node
  n_corr_nodes -= 1;

  int size = buffer.size();
  for(int rank=n_ranks-n_corr_nodes; rank<n_ranks; rank++)
    MPI_Send(&size, 1, MPI_INT32, rank, MPI_TAG_DELAY_TABLE, MPI_COMM_WORLD);
  MPI_Bcast(&buffer[0], size, MPI_PACKED, RANK_MANAGER_NODE, MPI_COMM_CORR_NODES);
}

void
MPI_Transfer::
unpack(std::vector<char> &buffer, Delay_table_akima &table, int sn[2]) {
  int size = buffer.size();
  int position = 0;

  // First, get the station number
  MPI_Unpack(&buffer[0], size, &position, sn, 2, MPI_INT32, MPI_COMM_WORLD);

  // Get all sources
  int32_t n_sources;
  MPI_Unpack(&buffer[0], size, &position, &n_sources, 1, MPI_INT32, MPI_COMM_WORLD);
  table.sources.resize(n_sources);
  for(int j = 0; j < n_sources ; j++){
    char source[81];
    MPI_Unpack(&buffer[0], size, &position, source, 81, MPI_CHAR, MPI_COMM_WORLD);
    table.sources[j] = source;
  }

  // Get all scans
  int32_t n_scans;
  MPI_Unpack(&buffer[0], size, &position, &n_scans, 1, MPI_INT32, MPI_COMM_WORLD);
  table.scans.resize(n_scans);
  for(int i = 0; i < n_scans; i++){
    Delay_table_akima::Scan &scan = table.scans[i];

    int64_t ticks;
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    scan.begin.set_clock_ticks(ticks);
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    scan.end.set_clock_ticks(ticks);

    MPI_Unpack(&buffer[0], size, &position, &scan.source, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.times, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.delays, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.phases, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.amplitudes, 1, MPI_INT32, MPI_COMM_WORLD);
  }
  // Get all times
  int32_t n_times;
  MPI_Unpack(&buffer[0], size, &position, &n_times, 1, MPI_INT32, MPI_COMM_WORLD);
  table.times.resize(n_times);
  MPI_Unpack(&buffer[0], size, &position, &table.times[0], n_times, MPI_DOUBLE, MPI_COMM_WORLD);
  // Get all delays
  int32_t n_delays;
  MPI_Unpack(&buffer[0], size, &position, &n_delays, 1, MPI_INT32, MPI_COMM_WORLD);
  table.delays.resize(n_delays);
  MPI_Unpack(&buffer[0], size, &position, &table.delays[0], n_delays, MPI_DOUBLE, MPI_COMM_WORLD);
  table.phases.resize(n_delays);
  MPI_Unpack(&buffer[0], size, &position, &table.phases[0], n_delays, MPI_DOUBLE, MPI_COMM_WORLD);
  table.amplitudes.resize(n_delays);
  MPI_Unpack(&buffer[0], size, &position, &table.amplitudes[0], n_delays, MPI_DOUBLE, MPI_COMM_WORLD);
  // Get all clocks
  int32_t n_clocks;
  MPI_Unpack(&buffer[0], size, &position, &n_clocks, 1, MPI_INT32, MPI_COMM_WORLD);
  table.clock_starts.resize(n_clocks);
  for (int i = 0; i < n_clocks; i++) {
    int64_t ticks;
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    table.clock_starts[i].set_clock_ticks(ticks);
  }
  table.clock_offsets.resize(n_clocks);
  MPI_Unpack(&buffer[0], size, &position, &table.clock_offsets[0], n_clocks, MPI_DOUBLE, MPI_COMM_WORLD);
  table.clock_epochs.resize(n_clocks);
  for (int i = 0; i < n_clocks; i++) {
    int64_t ticks;
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    table.clock_epochs[i].set_clock_ticks(ticks);
  }
  table.clock_rates.resize(n_clocks);
  MPI_Unpack(&buffer[0], size, &position, &table.clock_rates[0], n_clocks, MPI_DOUBLE, MPI_COMM_WORLD);

  SFXC_ASSERT(position == size);

  table.scan_nr = 0;
  table.acc.resize(0);
  table.acc_ph.resize(0);
  table.acc_amp.resize(0);
  table.splineakima.resize(0);
  table.splineakima_ph.resize(0);
  table.splineakima_amp.resize(0);
  table.initialise_next_scan();
  //std::cout << RANK_OF_NODE << " : receives all data (" << size << " bytes) " << "\n";
}

void
MPI_Transfer::
receive(MPI_Status &status, Delay_table_akima &table, int &station_nr) {
  MPI_Status status2;

  int size, sn[2];
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  std::vector<char> buffer(size);

  MPI_Recv(&buffer[0], size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  unpack(buffer, table, sn);
  station_nr = sn[0];
}

void
MPI_Transfer::
receive_bcast(MPI_Status &status, Delay_table_akima &table, int sn[2]) {
  MPI_Status status2;

  int size;
  MPI_Recv(&size, 1, MPI_INT32, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  std::vector<char> buffer(size);
  MPI_Bcast(&buffer[0], size, MPI_PACKED, RANK_MANAGER_NODE, MPI_COMM_CORR_NODES);
  unpack(buffer, table, sn);
}

void
MPI_Transfer::
pack(std::vector<char> &buffer, Uvw_model &table, int sn) {
  int32_t n_sources = table.sources.size();
  int32_t n_scans = table.scans.size();
  int32_t n_times = table.times.size();
  int32_t n_model = table.u.size();
  int32_t size = 5 * sizeof(int32_t) + n_sources * 81 * sizeof(char) +
                 n_scans * (2 * sizeof(int64_t) + 3 * sizeof(int32_t)) +
                 (n_times + 3 * n_model) * sizeof(double);

  buffer.resize(size);
  int position=0;

  // First integer is the station number
  MPI_Pack(&sn, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  // all sources
  MPI_Pack(&n_sources, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  for(int i = 0; i < n_sources; i++){
    char source[81];
    strncpy(source, table.sources[i].c_str(), 80);
    source[80] = 0;
    MPI_Pack(source, 81, MPI_CHAR, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  // all scans
  MPI_Pack(&n_scans, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  for(int i = 0; i < n_scans; i++){
    Uvw_model::Scan &scan = table.scans[i];
    int64_t ticks = scan.begin.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
    ticks = scan.end.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.source, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.times, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
    MPI_Pack(&scan.model_index, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  }
  // all times
  MPI_Pack(&n_times, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.times[0], n_times, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  // the model
  MPI_Pack(&n_model, 1, MPI_INT32, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.u[0], n_model, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.v[0], n_model, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);
  MPI_Pack(&table.w[0], n_model, MPI_DOUBLE, &buffer[0], size, &position, MPI_COMM_WORLD);

  SFXC_ASSERT(position == size);
//  std::cout << "sending " << size << " bytes of data " << "\n";
}

void
MPI_Transfer::
send(Uvw_model &table, int sn, int rank) {
  std::vector<char> buffer;
  pack(buffer, table, sn);
  int size = buffer.size();
  MPI_Send(&buffer[0], size, MPI_PACKED, rank, MPI_TAG_UVW_TABLE, MPI_COMM_WORLD);
}

void
MPI_Transfer::
bcast_corr_nodes(Uvw_model &table, int sn) {
  std::vector<char> buffer;
  pack(buffer, table, sn);
  int n_ranks, n_corr_nodes;
  MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
  MPI_Comm_size(MPI_COMM_CORR_NODES, &n_corr_nodes);
  // NB:: MPI_COMM_CORR_NODES includes the management node
  n_corr_nodes -= 1;

  int size = buffer.size();
  for(int rank=n_ranks-n_corr_nodes; rank<n_ranks; rank++)
    MPI_Send(&size, 1, MPI_INT32, rank, MPI_TAG_UVW_TABLE, MPI_COMM_WORLD);
  MPI_Bcast(&buffer[0], size, MPI_PACKED, RANK_MANAGER_NODE, MPI_COMM_CORR_NODES);
}

void
MPI_Transfer::
unpack(std::vector<char> &buffer, Uvw_model &table, int &sn) {
  int size = buffer.size();
  int position = 0;

  // First, get the station number
  MPI_Unpack(&buffer[0], size, &position, &sn, 1, MPI_INT32, MPI_COMM_WORLD);

  // Get all sources
  int32_t n_sources;
  MPI_Unpack(&buffer[0], size, &position, &n_sources, 1, MPI_INT32, MPI_COMM_WORLD);
  table.sources.resize(n_sources);
  for(int j = 0; j < n_sources ; j++){
    char source[81];
    MPI_Unpack(&buffer[0], size, &position, source, 81, MPI_CHAR, MPI_COMM_WORLD);
    table.sources[j] = source;
  }

  // Get all scans
  int32_t n_scans;
  MPI_Unpack(&buffer[0], size, &position, &n_scans, 1, MPI_INT32, MPI_COMM_WORLD);
  table.scans.resize(n_scans);
  for(int i = 0; i < n_scans; i++){
    Uvw_model::Scan &scan = table.scans[i];

    int64_t ticks;
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    scan.begin.set_clock_ticks(ticks);
    MPI_Unpack(&buffer[0], size, &position, &ticks, 1, MPI_INT64, MPI_COMM_WORLD);
    scan.end.set_clock_ticks(ticks);

    MPI_Unpack(&buffer[0], size, &position, &scan.source, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.times, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(&buffer[0], size, &position, &scan.model_index, 1, MPI_INT32, MPI_COMM_WORLD);
  }
  // Get all times
  int32_t n_times;
  MPI_Unpack(&buffer[0], size, &position, &n_times, 1, MPI_INT32, MPI_COMM_WORLD);
  table.times.resize(n_times);
  MPI_Unpack(&buffer[0], size, &position, &table.times[0], n_times, MPI_DOUBLE, MPI_COMM_WORLD);
  // Get all delays
  int32_t n_model;
  MPI_Unpack(&buffer[0], size, &position, &n_model, 1, MPI_INT32, MPI_COMM_WORLD);
  table.u.resize(n_model);
  table.v.resize(n_model);
  table.w.resize(n_model);
  MPI_Unpack(&buffer[0], size, &position, &table.u[0], n_model, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(&buffer[0], size, &position, &table.v[0], n_model, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(&buffer[0], size, &position, &table.w[0], n_model, MPI_DOUBLE, MPI_COMM_WORLD);

  SFXC_ASSERT(position == size);

  table.scan_nr = 0;
  table.acc_u.resize(0);
  table.acc_v.resize(0);
  table.acc_w.resize(0);
  table.splineakima_u.resize(0);
  table.splineakima_v.resize(0);
  table.splineakima_w.resize(0);
  table.initialise_spline_for_next_scan();
  //std::cout << RANK_OF_NODE << " : receives all data (" << size << " bytes) " << "\n";
}

void
MPI_Transfer::
receive(MPI_Status &status, Uvw_model &table, int &sn) {
  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  std::vector<char> buffer(size);

  MPI_Recv(&buffer[0], size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  unpack(buffer, table, sn);
}

void
MPI_Transfer::
receive_bcast(MPI_Status &status, Uvw_model &table, int &sn) {
  MPI_Status status2;
  int size;
  MPI_Recv(&size, 1, MPI_INT32, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  std::vector<char> buffer(size);
  MPI_Bcast(&buffer[0], size, MPI_PACKED, RANK_MANAGER_NODE, MPI_COMM_CORR_NODES);
  unpack(buffer, table, sn);
}

void
MPI_Transfer::send(Pulsar_parameters &pulsar_param, int rank) {
  int polyco_params_fixed_size = (11+10+6)*sizeof(char)+5*sizeof(double)+5*sizeof(float)+2*sizeof(int32_t);
  int size = 0;

  // first count all the data
  int32_t npulsar = pulsar_param.pulsars.size();
  size += sizeof(int32_t);
  std::map<std::string, Pulsar_parameters::Pulsar>::iterator it = pulsar_param.pulsars.begin();
  while(it!=pulsar_param.pulsars.end()){
    size += 11*sizeof(char) + sizeof(int32_t)+2*sizeof(double);
    size += sizeof(int32_t); // because we send the number of polyco tables
    std::vector<Pulsar_parameters::Polyco_params>::iterator poly = it->second.polyco_params.begin();
    while(poly != it->second.polyco_params.end()){
      size += polyco_params_fixed_size + poly->n_coef*sizeof(double);
      poly++;
    }
    it++;
  }

  // pack all data
  int position = 0;
  char message_buffer[size];

  MPI_Pack(&npulsar, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);
  it = pulsar_param.pulsars.begin();
  while(it!=pulsar_param.pulsars.end()){
    Pulsar_parameters::Pulsar &cur = it->second;
    MPI_Pack(&cur.name[0], 11, MPI_CHAR, message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&cur.nbins, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&cur.interval.start, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&cur.interval.stop, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
    int32_t npolyco = cur.polyco_params.size();
    MPI_Pack(&npolyco, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);

    std::vector<Pulsar_parameters::Polyco_params>::iterator poly = cur.polyco_params.begin();

    while(poly != cur.polyco_params.end()){
      MPI_Pack(&poly->name[0], 11, MPI_CHAR, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->date[0], 10, MPI_CHAR, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->utc, 1, MPI_FLOAT, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->tmid, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->DM, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->doppler, 1, MPI_FLOAT, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->residual, 1, MPI_FLOAT, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->ref_phase, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->ref_freq, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->site[0], 6, MPI_CHAR, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->data_span, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->n_coef, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->obs_freq, 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->bin_phase[0], 1, MPI_FLOAT, message_buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(&poly->bin_phase[1], 1, MPI_FLOAT, message_buffer, size, &position, MPI_COMM_WORLD);
      for(int i=0;i<poly->n_coef;i++){
        MPI_Pack(&poly->coef[i], 1, MPI_DOUBLE, message_buffer, size, &position, MPI_COMM_WORLD);
      }
      poly++;
    }
    it++;
  }

  // And finally send the data
  SFXC_ASSERT(position == size);
  MPI_Send(message_buffer, position, MPI_PACKED, rank, MPI_TAG_PULSAR_PARAMETERS, MPI_COMM_WORLD);
}

void
MPI_Transfer::receive(MPI_Status &status, Pulsar_parameters &pulsar_param) {
  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int32_t npulsars;
  int position = 0;
  MPI_Unpack(buffer, size, &position, &npulsars, 1, MPI_INT32, MPI_COMM_WORLD);

  for(int i=0; i<npulsars; i++){
    Pulsar_parameters::Pulsar newPulsar;
    MPI_Unpack(buffer, size, &position, &newPulsar.name[0], 11, MPI_CHAR, MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &newPulsar.nbins, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &newPulsar.interval.start, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &newPulsar.interval.stop,  1, MPI_DOUBLE, MPI_COMM_WORLD);
    int32_t npolyco;
    MPI_Unpack(buffer, size, &position, &npolyco, 1, MPI_INT32, MPI_COMM_WORLD);
    newPulsar.polyco_params.resize(npolyco);
    for(int j=0;j<npolyco;j++){
      Pulsar_parameters::Polyco_params *poly = &newPulsar.polyco_params[j];
      MPI_Unpack(buffer, size, &position, &poly->name[0], 11, MPI_CHAR, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->date[0], 10, MPI_CHAR, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->utc, 1, MPI_FLOAT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->tmid, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->DM, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->doppler, 1, MPI_FLOAT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->residual, 1, MPI_FLOAT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->ref_phase, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->ref_freq, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->site[0], 6, MPI_CHAR, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->data_span, 1, MPI_INT32, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->n_coef, 1, MPI_INT32, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->obs_freq, 1, MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->bin_phase[0], 1, MPI_FLOAT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, &poly->bin_phase[1], 1, MPI_FLOAT, MPI_COMM_WORLD);
      poly->coef.resize(poly->n_coef);
      for(int k=0;k<poly->n_coef;k++){
        MPI_Unpack(buffer, size, &position, &poly->coef[k], 1, MPI_DOUBLE, MPI_COMM_WORLD);
      }
    }
    std::string key(&newPulsar.name[0]);
    pulsar_param.pulsars.insert(std::pair<std::string,Pulsar_parameters::Pulsar>(key,newPulsar));
  }
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::send(std::set<std::string> &sources, int rank) {
  int size = 0;

  // first count all the data
  int32_t nsources = sources.size();
  size += sizeof(int32_t);
  std::set<std::string>::iterator it = sources.begin();
  while(it!=sources.end()){
    size += sizeof(int32_t) + (it->size() + 1) * sizeof(char);
    it++;
  }

  // pack all data
  int position = 0;
  char message_buffer[size];

  MPI_Pack(&nsources, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);
  it = sources.begin();
  while(it!=sources.end()){
    int32_t length = it->size() + 1;
    MPI_Pack(&length, 1, MPI_INT32, message_buffer, size, &position, MPI_COMM_WORLD);  
    MPI_Pack((void *)it->c_str(), length, MPI_CHAR, message_buffer, size, &position, MPI_COMM_WORLD);
    it++;
  }

  // And finally send the data
  SFXC_ASSERT(position == size);
  MPI_Send(message_buffer, position, MPI_PACKED, rank, MPI_TAG_SOURCE_LIST, MPI_COMM_WORLD);
}

void
MPI_Transfer::receive(MPI_Status &status, std::map<std::string, int> &sources){
  MPI_Status status2;

  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int32_t nsources;
  int position = 0;
  sources.clear();
  MPI_Unpack(buffer, size, &position, &nsources, 1, MPI_INT32, MPI_COMM_WORLD);

  for(int i=0; i<nsources; i++){
    int32_t source_len;
    char source[source_len];
    MPI_Unpack(buffer, size, &position, &source_len, 1, MPI_INT32, MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &source[0], source_len, MPI_CHAR, MPI_COMM_WORLD);
    sources[std::string(source)] = i;
    std::cout << "Added " << source << ", nr = " << i << "\n";
  }
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::send(Input_node_parameters &input_node_param, int rank) {
  int size = 0;
  size = 6 * sizeof(int32_t) + 3 * sizeof(int64_t);
  for (Input_node_parameters::Channel_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++) {
    size += 2 * sizeof(char) + sizeof(int32_t) * (3 + channel->tracks.size());
  }

  int position = 0;
  int32_t length;
  char message_buffer[size];

  MPI_Pack(&input_node_param.n_tracks, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.track_bit_rate, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.frame_size, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.fft_size, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  int64_t ticks = input_node_param.integr_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  ticks = input_node_param.offset.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&input_node_param.data_modulation, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  ticks = input_node_param.phasecal_integr_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);

  length = (int32_t)input_node_param.channels.size();
  MPI_Pack(&length, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  for (Input_node_parameters::Channel_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++) {
    MPI_Pack(&channel->bits_per_sample, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    // Tracks 
    length = (int32_t)channel->tracks.size();
    MPI_Pack(&length, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&channel->tracks[0], length, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);

    MPI_Pack(&channel->sideband, 1, MPI_CHAR,
	     message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&channel->polarisation, 1, MPI_CHAR,
	     message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&channel->frequency_number, 1, MPI_INT32,
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
             &input_node_param.n_tracks, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.track_bit_rate, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.frame_size, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.fft_size, 1, MPI_INT32,
             MPI_COMM_WORLD);
  int64_t ticks;
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  input_node_param.integr_time.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  input_node_param.offset.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &input_node_param.data_modulation, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  input_node_param.phasecal_integr_time.set_clock_ticks(ticks);
  int32_t n_channels;
  MPI_Unpack(buffer, size, &position,
             &n_channels, 1, MPI_INT32,
             MPI_COMM_WORLD);
  while (n_channels > 0) {
    Input_node_parameters::Channel_parameters channel;
    MPI_Unpack(buffer, size, &position, 
               &channel.bits_per_sample, 1, MPI_INT32,
               MPI_COMM_WORLD);
    // Tracks 
    MPI_Unpack(buffer, size, &position,
               &length, 1, MPI_INT32,
               MPI_COMM_WORLD);
    int tracks[length];
    MPI_Unpack(buffer, size, &position,
               &tracks, length, MPI_INT32,
               MPI_COMM_WORLD);
    for (int i=0; i<length; i++) {
      channel.tracks.push_back(tracks[i]);
    }

    MPI_Unpack(buffer, size, &position,
	       &channel.sideband, 1, MPI_CHAR,
	       MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
	       &channel.polarisation, 1, MPI_CHAR,
	       MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
	       &channel.frequency_number, 1, MPI_INT32,
	       MPI_COMM_WORLD);

    input_node_param.channels.push_back(channel);

    n_channels--;
  }
  SFXC_ASSERT(position == size);
}

void
MPI_Transfer::send(Correlation_parameters &corr_param, int rank) {
  int size = 0;
  size =
    4*sizeof(int64_t) + 13*sizeof(int32_t) + sizeof(int64_t) +
    3*sizeof(char) + corr_param.station_streams.size() * (5 * sizeof(int32_t) + 3 * sizeof(int64_t) + sizeof(char)) +
    11*sizeof(char);
  int position = 0;
  char message_buffer[size];

  int64_t ticks = corr_param.start_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  ticks = corr_param.stop_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  ticks = corr_param.integration_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  ticks = corr_param.sub_integration_time.get_clock_ticks();
  MPI_Pack(&ticks, 1, MPI_INT64,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.number_channels, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.fft_size_delaycor, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.fft_size_correlation, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.window, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.integration_nr, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.slice_nr, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.slice_offset, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&corr_param.sample_rate, 1, MPI_INT32,
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

  MPI_Pack(&corr_param.n_phase_centers, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.pulsar_binning, 1, MPI_INT32,
           message_buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&corr_param.source[0], 11, MPI_CHAR,
           message_buffer, size, &position, MPI_COMM_WORLD);

  for (Correlation_parameters::Station_iterator station =
         corr_param.station_streams.begin();
       station != corr_param.station_streams.end(); station++) {
    MPI_Pack(&station->station_number, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->station_stream, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    ticks = station->start_time.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64,
             message_buffer, size, &position, MPI_COMM_WORLD);
    ticks = station->stop_time.get_clock_ticks();
    MPI_Pack(&ticks, 1, MPI_INT64,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->bits_per_sample, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->sample_rate, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->channel_freq, 1, MPI_INT64,
	     message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->bandwidth, 1, MPI_INT32,
             message_buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&station->sideband, 1, MPI_CHAR,
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
  int64_t ticks;
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  corr_param.start_time.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  corr_param.stop_time.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  corr_param.integration_time.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &ticks, 1, MPI_INT64,
             MPI_COMM_WORLD);
  corr_param.sub_integration_time.set_clock_ticks(ticks);
  MPI_Unpack(buffer, size, &position,
             &corr_param.number_channels, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.fft_size_delaycor, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.fft_size_correlation, 1, MPI_INT32,
             MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.window, 1, MPI_INT32,
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

  MPI_Unpack(buffer, size, &position,
             &corr_param.n_phase_centers, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
             &corr_param.pulsar_binning, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position,
               &corr_param.source[0], 11, MPI_CHAR, MPI_COMM_WORLD);

  while (position < size) {

    Correlation_parameters::Station_parameters station_param;
    MPI_Unpack(buffer, size, &position,
               &station_param.station_number, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.station_stream, 1, MPI_INT32,
               MPI_COMM_WORLD);
    int64_t ticks;
    MPI_Unpack(buffer, size, &position,
               &ticks, 1, MPI_INT64,
               MPI_COMM_WORLD);
    station_param.start_time.set_clock_ticks(ticks);
    MPI_Unpack(buffer, size, &position,
               &ticks, 1, MPI_INT64,
               MPI_COMM_WORLD);
    station_param.stop_time.set_clock_ticks(ticks);
    MPI_Unpack(buffer, size, &position,
               &station_param.bits_per_sample, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.sample_rate, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
	       &station_param.channel_freq, 1, MPI_INT64,
	       MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.bandwidth, 1, MPI_INT32,
               MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position,
               &station_param.sideband, 1, MPI_CHAR,
               MPI_COMM_WORLD);
    corr_param.station_streams.push_back(station_param);
  }

  SFXC_ASSERT(position == size);
}
