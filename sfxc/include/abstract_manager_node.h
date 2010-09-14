/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: Node.h 251 2007-06-12 13:56:30Z kruithof $
 *
 */

#ifndef ABSTRACT_MANAGER_NODE_H
#define ABSTRACT_MANAGER_NODE_H

#include "node.h"
#include "control_parameters.h"
#include "delay_table_akima.h"
#include "uvw_model.h"

#include <queue>

class Connexion_params
{
	public:
		/// list of all listening ip/port
		std::vector<uint64_t> ip_port_;
};

/** Abstract manager node which defines generic functions needed by
    manager nodes.
 **/
class Abstract_manager_node : public Node {
public:
//   Abstract_manager_node(int rank, int numtasks,
//                         const Control_parameters &param);
  Abstract_manager_node(int rank, int numtasks,
                        Log_writer *writer,
                        const Control_parameters &param);
  virtual ~Abstract_manager_node();

  void start_input_node(int rank, const std::string &station);
  void start_output_node(int rank);
  void start_correlator_node(int rank);
  void start_log_node(int rank);
  void start_log_node(int rank, char *filename);

  void end_node(int rank);

  int get_status(int rank);
  void set_message_level(int rank, int32_t messagelevel);

  /* set Data_readers */
  // for files
  void set_data_reader(int rank, int stream_nr,
                       const std::string &filename);
  // for tcp
  void set_TCP(int writer_rank, int writer_stream_nr,
               int reader_rank, int reader_stream_nr);


	// for tcp
	// request that hte reader connect to the writer (this is simplified)
	void connect_to( int writer_rank,
									 int writer_stream_nr,
                   int reader_rank,
                   int reader_stream_nr,
                   std::vector<uint64_t>& params, int rank, MPI_Request* req );

	void connect_writer_to( int writer_rank,
									 int writer_stream_nr,
                   int reader_rank,
                   int reader_stream_nr,
                   std::vector<uint64_t>& params, int rank, MPI_Request* req );

  // for void
  void set_data_writer_void(int writer_rank, int writer_stream_nr);

  // ...

  /* set Data_writers */
  // for files
  void set_data_writer(int rank, int stream_nr, const std::string &filename);

  /// Interface to Input node

  // Sets the track parameters for a station:
  void input_node_set(const std::string &station,
                      Input_node_parameters &input_node_params);
  /// Returns the time in milliseconds since midnight on the start-day
  Time input_node_get_current_time(const std::string &station);
  void input_node_set_time(const std::string &station,
                           Time start_time, Time stop_time);

  // Send a new time slice, start and stop time are in milliseconds
  void input_node_set_time_slice(const std::string &station, int32_t channel,
                                 int32_t stream_nr,
                                 Time start_time, Time stop_time);


  void output_node_set_global_header(char* header_msg, int size);

  int get_number_of_processes() const;
  const Control_parameters &get_control_parameters() const;

  size_t number_correlator_nodes() const;

  size_t input_node(const std::string &station) const;
  size_t input_rank(size_t input_node_nr) const;
  size_t input_rank(const std::string &station_name) const;

  int correlator_rank(int correlator);
  void correlator_node_set(Correlation_parameters &parameters,
                           int corr_node_nr);
  void correlator_node_set_all(Delay_table_akima &delay_table,
                               const std::string &station_name);
  void correlator_node_set_all(Uvw_model &uvw_table,
                               const std::string &station_name);
  void correlator_node_set_all(Pulsar_parameters &pulsar);

  void set_correlator_node_ready(size_t correlator_rank, bool ready=true);

  void send(Delay_table_akima &delay_table, int station, int to_rank);

  size_t get_channel(const std::string &channel);

  const std::map<std::string, int> &get_input_node_map() const;

  Time integration_time() const {
    return integration_time_;
  }

  // Override functions from node to be able to catch the assertion message
  Node::MESSAGE_RESULT check_and_process_waiting_message();
  Node::MESSAGE_RESULT process_all_waiting_messages();
  Node::MESSAGE_RESULT check_and_process_message();

  // Terminate all the nodes after an assertion has been raised in a node
  void terminate_nodes_after_assertion(int calling_node);

protected:
  void wait_for_setting_up_channel(int rank);

  // Data
  Control_parameters control_parameters;
  Pulsar_parameters pulsar_parameters;
  int numtasks;

  // Map from a station name to the Input_node number
  std::map<std::string, int> input_node_map;
  // Map from the input node number to the MPI_rank
  std::vector<int> input_node_rank;

	// stores the connexion parameters to the input nodes
  std::vector<Connexion_params*> input_node_cnx_params_;
	std::vector<Connexion_params*> output_node_cnx_params_;

  // Map from the correlator node number to the MPI_rank
  std::vector<int> correlator_node_rank;

  Time integration_time_;

#ifdef SFXC_DETERMINISTIC
  /// Status of the correlation node
  std::vector<bool> correlator_node_ready;
#else
  std::queue<int> ready_correlator_nodes;
#endif
};

#endif // ABSTRACT_MANAGER_NODE_H
