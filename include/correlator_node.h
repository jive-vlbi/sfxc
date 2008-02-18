/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H

#include "node.h"

#include "multiple_data_readers_controller.h"
#include "single_data_writer_controller.h"

#include "semaphore_buffer.h"

#include "control_parameters.h"
#include "bits_to_float_converter.h"

#include "log_writer_mpi.h"
#include "correlation_core.h"
#include "delay_correction.h"
#include "tasklet/tasklet_manager.h"
#include "integer_delay_correction.h"

#include "timer.h"

// Declare the correlator controller:
class Correlator_node;

/**
 * Correlator_node_controller processes specific signals for the Correlator node.
 **/ 
class Correlator_node_controller : public Controller
{
public:
  Correlator_node_controller(Correlator_node &node);
  ~Correlator_node_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Correlator_node &node;
};

 /**
  * A correlate node will initialize the correlation process and connect
  * to the output node. It can receive messages from a data node asking to
  * open an input connection and from the controller node to process a
  * time slice. After the slice is processed the node will send a message
  * to the controller node saying it is available for a next job.
  * 
  * \ingroup Node
  **/
class Correlator_node : public Node
{
public:
  typedef Correlator_node                              Self;
  typedef Multiple_data_readers_controller::value_type Input_buffer_element;
  typedef Semaphore_buffer<Input_buffer_element>       Input_buffer;
  typedef boost::shared_ptr<Input_buffer>              Input_buffer_ptr;
  typedef Buffer_element_vector<char>                  output_value_type;
  
private:
  typedef boost::shared_ptr<Bits_to_float_converter>  Bits2float_ptr;
  typedef boost::shared_ptr<Delay_correction>         Delay_correction_ptr;
public:
  enum Status {
    // Initialise the Correlate node
    STOPPED=0,
    // The node is correlating
    CORRELATING,
    END_CORRELATING
  };
  
  enum CORRELATE_STEPS {
    /// Initialise the correlator for a new time slice:
    INITIALISE_TIME_SLICE=0,
    /// Do one integration step:
    CORRELATE_INTEGRATION_SLICE,
    /// Finish processing a time slice:
    END_TIME_SLICE
  };
  
  Correlator_node(int rank, int nr_corr_node);
  ~Correlator_node();
  
  void start();

  void output_node_set_timeslice(int slice_nr, int stream_nr, int bytes);

  /// Callback function for adding a data_reader:
  void hook_added_data_reader(size_t reader);
  /// Callback function for adding a data_writer:
  void hook_added_data_writer(size_t writer);

  void add_delay_table(int sn, Delay_table_akima &table);

  void set_parameters(const Correlation_parameters &parameters);
  

  int get_correlate_node_number();

  /** Number of integration steps done in the current time slice **/
  int number_of_integration_steps_in_time_slice();

  /** Size in bytes of the output of one integration step **/
  int output_size_of_one_integration_step();
private:
  void correlate();

  
private:
  Correlator_node_controller       correlator_node_ctrl;
  Multiple_data_readers_controller data_readers_ctrl;
  Single_data_writer_controller    data_writer_ctrl;
  
  // State variables:
  CORRELATE_STEPS correlate_state;
  Status status;

  /// Number of the correlator node
  int nr_corr_node;

  std::vector< Bits2float_ptr >               bits2float_converters;
  std::vector< Delay_correction_ptr >         delay_modules;
  Correlation_core                            correlation_core;
  
  int n_integration_slice_in_time_slice;
  
  Timer bits_to_float_timer_, delay_timer_, correlation_timer_;
};

#endif // CORRELATOR_NODE_H
