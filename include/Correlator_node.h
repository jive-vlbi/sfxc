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

#include <Node.h>

#include <Multiple_data_readers_controller.h>
#include <Single_data_writer_controller.h>

#include <Integration_slice.h>
#include <Semaphore_buffer.h>

#include "Log_writer_mpi.h"

/// TODO: NGHK: REMOVE
#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
extern RunP  RunPrms;
extern GenP  GenPrms;
extern StaP  StaPrms[NstationsMax];



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
  typedef Correlator_node                Self;
  typedef Buffer_element<char,131072>    input_value_type;
  typedef Buffer_element<char,131072>    output_value_type;

  enum STATUS {
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
  
  // Functions for multithreading init reader:
  struct Init_reader_struct {
    Self *corr_node;
    pthread_t thread;
    int64_t sn, startIS;
  };
  static void *start_init_reader(void *);
  
  
  Correlator_node(int rank, int nr_corr_node, int buff_size);
  ~Correlator_node();
  
  void start();

  /// Starts the correlation process.  
  void start_correlating(int64_t start, int64_t duration);

  /// Callback function for adding a data_reader:
  void hook_added_data_reader(size_t reader);
  /// Callback function for adding a data_writer:
  void hook_added_data_writer(size_t writer);

  void add_delay_table(int sn, DelayTable &table);
    

  /// Get the Integration_slice (the class doing the actual work)
  Integration_slice &get_integration_slice() {
    return integration_slice;
  }
  Data_writer &get_data_writer() {
    return get_integration_slice().get_data_writer();
  }
  
  int get_correlate_node_number();
  void set_parameters(RunP &runPrms, GenP &genPrms, StaP *staPrms);
  
  void set_slice_number(int sliceNr);

  /** Number of integration steps done in the current time slice **/
  int number_of_integration_steps_in_time_slice();

  /** Size in bytes of the output of one integration step **/
  int output_size_of_one_integration_step();
private:
  // Buffer for the output, input is directly handled by the Correlator_controller
  Semaphore_buffer<output_value_type> output_buffer;

  Correlator_node_controller       correlator_node_ctrl;
  Multiple_data_readers_controller data_readers_ctrl;
  Single_data_writer_controller    data_writer_ctrl;

  // The actual correlator code:
  Integration_slice integration_slice;
  
  // State variables:
  CORRELATE_STEPS correlate_state;
  STATUS status;
  /// Number of elements in a buffer
  int                                          buffer_size, nr_corr_node;

  int64_t startIS;
  int sliceNr;
};

#endif // CORRELATOR_NODE_H
