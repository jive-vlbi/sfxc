/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef MPI_TRANSFER_H_
#define MPI_TRANSFER_H_

#include <set>

#include "types.h"
#include "sfxc_mpi.h"
#include "delay_table_akima.h"
#include "uvw_model.h"

#include "control_parameters.h"

/** Transfer special classes for the sfxc using MPI
 **/
class MPI_Transfer {
public:
  MPI_Transfer();

  static void send(Delay_table &table, int sn, int rank);
  static void bcast_corr_nodes(Delay_table &table, int sn[2]);
  static void pack(std::vector<char> &buffer, Delay_table &table, int sn[2]);

  static void receive(MPI_Status &status, Delay_table &table, int &sn);
  static void receive_bcast(MPI_Status &status, Delay_table &table, int sn[2]);
  static void unpack(std::vector<char> &buffer, Delay_table &table, int sn[2]);
  
  static void send(Uvw_model &table, int sn, int rank);
  static void bcast_corr_nodes(Uvw_model &table, int sn);
  static void pack(std::vector<char> &buffer, Uvw_model &table, int sn);

  static void receive(MPI_Status &status, Uvw_model &table, int &sn);
  static void receive_bcast(MPI_Status &status, Uvw_model &table, int &sn);
  static void unpack(std::vector<char> &buffer, Uvw_model &table, int &sn);

  static void send(Pulsar_parameters &table, int rank);
  static void receive(MPI_Status &status, Pulsar_parameters &pulsar_param);

  static void send(Input_node_parameters &input_node_param, int rank);
  static void receive(MPI_Status &status, Input_node_parameters &input_node_param);

  static void send(Correlation_parameters &corr_param, int rank);
  static void receive(MPI_Status &status, Correlation_parameters &corr_param);

  static void bcast_corr_nodes(Mask_parameters &mask_param);
  static void pack(std::vector<char> &buffer, Mask_parameters &mask_param);
  static void receive_bcast(MPI_Status &status, Mask_parameters &mask_param);
  static void unpack(std::vector<char> &buffer, Mask_parameters &mask_param);

  static void send(std::set<std::string> &sources, int rank);
  static void receive(MPI_Status &status, std::map<std::string, int> &sources);

  static void send_ip_address(std::vector<uint64_t>& params, const int rank);
  static void receive_ip_address(std::vector<uint64_t>& params, std::string& hostname, const int rank);


  static void send_connect_writer_to_msg(const uint32_t info[4],
					 const std::vector<uint64_t>& params,
					 const std::string& hostname,
					 const int dstrank);
  static void recv_connect_writer_to_msg(uint32_t info[4],
					 std::vector<uint64_t>& params,
					 std::string& hostname,
					 const int srcrank );

  static void send_connect_to_msg(const uint32_t info[4],
				  const std::vector<uint64_t>& params,
				  const std::string& hostname,
				  const int dstrank);
  static void recv_connect_to_msg(uint32_t info[4],
				  std::vector<uint64_t>& params,
				  std::string& hostname,
				  const int srcrank);
};

#endif /*MPI_TRANSFER_H_*/
