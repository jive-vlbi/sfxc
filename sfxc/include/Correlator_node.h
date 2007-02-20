#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H

#include <Node.h>
#include <Correlator_controller.h>

#include <Multiple_data_readers_controller.h>
#include <Output_controller.h>

#include <Semaphore_buffer.h>

#include "Log_writer_mpi.h"

// Declare the correlator controller:
class Correlator_controller;

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
    /// Proceed to the initial position in the data stream:
    FIND_INITIAL_OFFSETS,
    /// Do one integration step:
    CORRELATE_SEGMENT,
    /// Finish processing a time slice:
    END_TIME_SLICE
  };
  
  Correlator_node(int rank, int buff_size=10);
  ~Correlator_node();
  
  void start();
  
  void add_data_reader(Data_reader *reader);
  void set_data_reader(int node, Data_reader *reader);
  
  /// Destroys the previous writer, if it exists.  
  void set_data_writer(Data_writer *data_writer);

  void start_correlating() { 
    status=CORRELATING; 
    correlate_state = FIND_INITIAL_OFFSETS; 
  }
  bool get_correlating() const { return (status==CORRELATING); }

private:
  std::vector<Data_reader *>     data_readers;
  // Buffer for the output, input is directly handled by the Correlator_controller
  Semaphore_buffer<output_value_type>      output_buffer;
  Data_writer                    *data_writer;

  Correlator_controller          correlator_controller;
  Output_controller              output_controller;
  
  // State variables:
  int correlate_state, status;
};

#endif // CORRELATOR_NODE_H
