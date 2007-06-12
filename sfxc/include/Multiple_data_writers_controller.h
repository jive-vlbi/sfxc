/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef MULTIPLE_DATA_WRITERS_CONTROLLER_H
#define MULTIPLE_DATA_WRITERS_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Buffer2data_writer.h>



class Multiple_data_writers_controller : public Controller {
  typedef Multiple_data_writers_controller  Self;
public:
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  typedef Buffer2data_writer<value_type>   Buffer2writer;
  
  Multiple_data_writers_controller(Node &node);
  ~Multiple_data_writers_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
  Buffer *buffer(unsigned int i);
  void set_buffer(unsigned int i, Buffer *buff);
  
  Buffer2writer *operator[](int i);
  
  bool ready();
  
  Data_writer *get_data_writer(int i);
    
  int get_rank_node_reader(int i) {
    assert((0<=i) && (i < (int)data_writers.size()));
    return data_writers[i].rank_node_reader;
  }  
  
  int get_stream_number_reader(int i) {
    assert((0<=i) && (i < (int)data_writers.size()));
    return data_writers[i].stream_number_reader;
  }  
  
private:
  class Output_stream {
  public:
    Output_stream() 
      : buffer2writer(NULL), rank_node_reader(-1), stream_number_reader(-1) {
    }
    /// The actual output stream
    // These are pointers, because a resize of the vector will 
    // copy construct all the elements and then destroy the old 
    // elements and we can't copy construct the extra threads.
    Buffer2writer *buffer2writer;
    
    /** The rank of the node the data is sent to, or -1 if the stream
     *  is connected to a file.
     **/
    int rank_node_reader;

    /** The number of the stream under which the data enters the data reader
     **/
    int stream_number_reader;
  };

  void add_data_writer(unsigned int i, Data_writer *writer, 
                       int rank_node_reader, int stream_number_reader);


  Buffer2writer &get_buffer2writer(unsigned int i);

  std::vector< Output_stream >  data_writers;
};

#endif /* MULTIPLE_DATA_WRITERS_CONTROLLER_H */
