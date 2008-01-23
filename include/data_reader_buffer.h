/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_BUFFER_H
#define DATA_READER_BUFFER_H

#include <boost/shared_ptr.hpp>
#include <assert.h>

#include "utils.h"
#include "data_reader.h"
#include "buffer.h"

//

/** Specialisation of Data_reader for reading from a buffer.
 **/
template < class Element = Buffer_element<char,131072> >
class Data_reader_buffer : public Data_reader {
  typedef Element                          value_type;
  typedef typename Element::value_type     element_type;
  typedef Buffer<value_type>               Buffer;
  
public:
  /** Constructor, reads from buffer
   **/
  Data_reader_buffer(boost::shared_ptr<Buffer> buff);

  ~Data_reader_buffer();

  bool eof();  

  boost::shared_ptr<Buffer> get_buffer() {
    return buffer;
  }
private:
  size_t do_get_bytes(size_t nElements, char *out);

  // The input buffer
  boost::shared_ptr<Buffer> buffer;
  // Number of bytes left in the current buffer-element
  int          bytes_left;
  element_type *data_start;
  // Is there more data arriving in the buffer:
  bool         end_of_file;
};

template <class Element>
Data_reader_buffer<Element>::Data_reader_buffer(boost::shared_ptr<Buffer> buff)
 : Data_reader(), 
   buffer(buff), bytes_left(0), 
   end_of_file(false)
{
  assert(buffer != NULL);
}

template <class Element>
Data_reader_buffer<Element>::~Data_reader_buffer() {
}

template <class Element>
size_t Data_reader_buffer<Element>::do_get_bytes(size_t nElements, char *out_) {
  element_type *out = (element_type *)out_;
  size_t elements_to_read = nElements;
  while (elements_to_read > 0) {
    if (bytes_left == 0) {
      if (buffer->empty()) {
        return nElements - elements_to_read;
      }
      data_start = buffer->consume(bytes_left).buffer();
      if (bytes_left == 0) {
        end_of_file = true;
        buffer->consumed();
        return nElements - elements_to_read;
      }
    }
    size_t curr_read = 
      (elements_to_read < (uint64_t)bytes_left ? elements_to_read : bytes_left);
    if (out != NULL) {
      memcpy(out, data_start, curr_read*sizeof(element_type));
      out += curr_read;
    }
    data_start += curr_read;
    elements_to_read -=curr_read;
    bytes_left -=curr_read;
    
    if (bytes_left == 0) {
      buffer->consumed();
    }
  }
  return nElements;
}

template <class Element>
bool Data_reader_buffer<Element>::eof() {
  return (end_of_file && (bytes_left == 0)); 
}

#endif // DATA_READER_BUFFER_H
