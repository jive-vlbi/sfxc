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

#include "utils.h"
#include "data_reader.h"

#include <boost/shared_ptr.hpp>

/** Specialisation of Data_reader for reading from a buffer.
 **/
template < class T >
class Data_reader_buffer : public Data_reader {
  typedef typename T::value_type         value_type;
  typedef typename T::Queue              Queue;
  typedef typename T::Queue_ptr          Queue_ptr;

public:
  /** Constructor, reads from buffer
   **/
  Data_reader_buffer(Queue_ptr queue);

  ~Data_reader_buffer();

  bool eof();

  bool can_read();

  Queue_ptr get_queue() {
    return queue;
  }
private:
  int do_get_bytes(size_t nElements, char *out);

  // The input buffer
  Queue_ptr    queue;
  value_type   data_start;
  // Number of bytes left in data_start
  int bytes_left;
  // Is there more data arriving in the buffer:
  bool         end_of_file;
};

template <class Element>
Data_reader_buffer<Element>::Data_reader_buffer(Queue_ptr queue)
    : Data_reader(),
    queue(queue), bytes_left(0),
    end_of_file(false) {
  SFXC_ASSERT(queue != Queue_ptr());
}

template <class Element>
Data_reader_buffer<Element>::~Data_reader_buffer() {}

template <class Element>
int Data_reader_buffer<Element>::do_get_bytes(size_t nElements, char *out) {
  SFXC_ASSERT(queue!=Queue_ptr());
  if (queue->empty()) return 0;
  size_t elements_to_read = nElements;
  while (elements_to_read > 0) {
    if (bytes_left == 0) {
      if (queue->empty()) {
        return nElements - elements_to_read;
      }
      // get a new buffer element
      data_start = queue->front();
      queue->pop();
      bytes_left = data_start.actual_size;
      SFXC_ASSERT(bytes_left >= 0);
    }
    if (bytes_left != 0) {
      // Copy the data
      size_t curr_read =
        (elements_to_read < (size_t)bytes_left ? elements_to_read : bytes_left);
      if (out != NULL) {
        memcpy(out, &(*data_start.data)[data_start.actual_size-bytes_left],
               curr_read);
        out += curr_read;
      }
      elements_to_read -=curr_read;
      bytes_left -=curr_read;

    }
    if (bytes_left == 0) {
      // get a release the buffer element
      data_start = value_type();
    }
  }
  return nElements;
}

template <class Element>
bool Data_reader_buffer<Element>::eof() {
  return (end_of_file && (bytes_left == 0));
}

template <class Element>
bool Data_reader_buffer<Element>::can_read() {
  DEBUG_MSG("Data_reader_buffer: can read not implemented");
  return true;
}

#endif // DATA_READER_BUFFER_H
