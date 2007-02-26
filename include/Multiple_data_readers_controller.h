/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writer_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef MULTIPLE_DATA_READERS_CONTROLLER_H
#define MULTIPLE_DATA_READERS_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Data_reader2buffer.h>



class Multiple_data_readers_controller : public Controller {
  typedef Multiple_data_readers_controller  Self;
public:
  /// TODO: NGHK: Make this type global?
  typedef Buffer_element<char,131072>      value_type;
  typedef Data_reader2buffer<value_type>   Reader2buffer;
  typedef Buffer<value_type>               Buffer;
  
  Multiple_data_readers_controller(Node &node);
  ~Multiple_data_readers_controller();

  Process_event_status process_event(MPI_Status &status);
  
  Buffer *get_buffer(unsigned int i);
  void set_buffer(unsigned int i, Buffer *buffer);

  std::vector<Data_reader *> &get_vector_data_readers();

  bool initialised(unsigned int i);
  unsigned int number_of_data_readers();
private:
  void add_data_reader(int i, Data_reader *reader);

  // These are pointers, because a resize of the vector will 
  // copy construct all the elements and then destroy the old 
  // elements and we can't copy construct the extra threads.
  std::vector< Data_reader2buffer<value_type>* >   data_readers;
  
  // InData expects a vector of input_readers.
  std::vector<Data_reader *>                     data_readers_out;
  
  bool more_data_readers_can_be_added;
};

#endif /* MULTIPLE_DATA_READERS_CONTROLLER_H */
