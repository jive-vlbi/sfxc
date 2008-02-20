/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef MULTIPLE_DATA_READERS_CONTROLLER_H
#define MULTIPLE_DATA_READERS_CONTROLLER_H

#include "controller.h"

#include "data_writer.h"
#include "semaphore_buffer.h"
#include "data_reader2buffer.h"
#include "data_reader_buffer.h"



class Multiple_data_readers_controller : public Controller {
  typedef Multiple_data_readers_controller  Self;
public:
  typedef Buffer_element<char,256>         value_type;
  typedef Data_reader2buffer<value_type>    Reader2buffer;
  typedef Buffer<value_type>                Buffer;
  typedef Data_reader_buffer<value_type>    Reader_buffer;
  
  Multiple_data_readers_controller(Node &node);
  ~Multiple_data_readers_controller();

  Process_event_status process_event(MPI_Status &status);

  /** Sets the buffer for an input stream and constructs a Data_reader_buffer
   * for it. **/    
  void set_buffer(unsigned int i, 
                  boost::shared_ptr<Buffer> buffer);

  boost::shared_ptr<Buffer> get_buffer(unsigned int i);

  /** Returns the data reader, which is either the real reader or the
   *  buffered reader if a buffer is set. **/
  boost::shared_ptr<Data_reader> get_data_reader(int i);
  
  
  /* Reader2buffer *get_data_reader2buffer(int i); */

  /* Returns true if data_reader(i) != NULL */
  bool initialised(unsigned int i);
  
  /** The number of input readers **/
  size_t number_of_data_readers();
private:

  void add_data_reader(int i, boost::shared_ptr<Data_reader> reader);

  // NGHK: TODO: Factorize into a single class?
  // These are pointers, because a resize of the vector will 
  // copy construct all the elements and then destroy the old 
  // elements and we can't copy construct the extra threads.
  std::vector< Reader2buffer * >        data_readers;
  std::vector< boost::shared_ptr<Reader_buffer> > 
                                        buffer_readers;
  std::vector< bool >                   reader_known;
};

#endif /* MULTIPLE_DATA_READERS_CONTROLLER_H */
