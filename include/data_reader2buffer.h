/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef DATA_READER2BUFFER_H
#define DATA_READER2BUFFER_H

#include <assert.h>
#include <pthread.h>
#include <boost/shared_ptr.hpp>

#include "data_reader.h"
#include "buffer.h"

/** Reads data from the data reader and puts it in a buffer,
 * which is useful for non-blocking IO.
 **/
template <class T>
class Data_reader2buffer {
  typedef Data_reader2buffer<T> Self;
public:
  typedef boost::shared_ptr< Data_reader > Data_reader_ptr;
  typedef boost::shared_ptr< Buffer<T> >   Buffer_ptr;

  enum State {
    STOPPED=0, ///< Not running, the additional thread is not active
    SUSPENDED, /**< Not running, the additional thread is waiting
                    (e.g. for a change of buffers) **/
    RUNNING    ///< The thread is writing data from the buffer
  };

  Data_reader2buffer();
  Data_reader2buffer(const Data_reader2buffer &buffer);
  ~Data_reader2buffer();

  boost::shared_ptr<Data_reader> get_data_reader();
  void set_data_reader(boost::shared_ptr<Data_reader> data_reader);

  boost::shared_ptr< Buffer<T> > get_buffer();
  void set_buffer(boost::shared_ptr< Buffer<T> > buffer);

  void start();
  void try_start();
  void stop();
  State get_state();
  void set_state(State new_state);

private:
  static void *start_reading(void *);
  void read();
private:
  Data_reader_ptr data_reader;
  Buffer_ptr      buffer;
  State           state;
  pthread_t       io_thread;

  pthread_mutex_t mutex_for_set_state;
};


// Implementation:
template <class T>
Data_reader2buffer<T>::Data_reader2buffer()
    : state(STOPPED) {
  pthread_mutex_init(&mutex_for_set_state, NULL);
}

template <class T>
Data_reader2buffer<T>::
Data_reader2buffer(const Data_reader2buffer &buffer) {
  // No copy constructor, the threads don't like it
  assert(false);
}

template <class T>
Data_reader2buffer<T>::~Data_reader2buffer() {
  stop();
}

template <class T>
void
Data_reader2buffer<T>::set_data_reader(Data_reader_ptr reader) {
  assert(state != RUNNING);
  assert(data_reader == Data_reader_ptr());
  data_reader = reader;
}

template <class T>
typename Data_reader2buffer<T>::Data_reader_ptr
Data_reader2buffer<T>::get_data_reader() {
  return data_reader;
}

template <class T>
void
Data_reader2buffer<T>::set_buffer(Buffer_ptr buff) {
  assert(buffer == Buffer_ptr());
  assert(state == STOPPED);
  buffer = buff;
}

template <class T>
typename Data_reader2buffer<T>::Buffer_ptr
Data_reader2buffer<T>::get_buffer() {
  return buffer;
}


template <class T>
void
Data_reader2buffer<T>::try_start() {
  if ((data_reader != Data_reader_ptr() ) && (buffer != Buffer_ptr())) {
    start();
  }
}

template <class T>
void
Data_reader2buffer<T>::start() {
  assert(data_reader != Data_reader_ptr());
  assert(buffer != Buffer_ptr());

  if (state == STOPPED) {
    set_state(RUNNING);
    pthread_create(&io_thread, NULL,
                   start_reading, static_cast<void*>(this));
  } else {
    set_state(RUNNING);
  }
}

template <class T>
void
Data_reader2buffer<T>::stop() {
  if (state == STOPPED) return;
  set_state(STOPPED);
  pthread_join(io_thread, NULL);
}

template <class T>
typename Data_reader2buffer<T>::State
Data_reader2buffer<T>::get_state() {
  return state;
}

template <class T>
void
Data_reader2buffer<T>::set_state(State new_state) {
  pthread_mutex_lock( &mutex_for_set_state );
  state = new_state;
  pthread_mutex_unlock( &mutex_for_set_state );
}

template <class T>
void *
Data_reader2buffer<T>::start_reading(void * self_) {
  Self *self = static_cast<Self *>(self_);
  self->read();
  return NULL;
}

template <class T>
void
Data_reader2buffer<T>::read() {
  while (state != STOPPED) {
    if (state == SUSPENDED) {
      usleep(100); // 100 microseconds
    } else {
      if (buffer->full()) {
        usleep(100); // 100 microseconds
      } else if (data_reader->eof()) {
        DEBUG_MSG("data_reader->eof()");
        set_state(STOPPED);
      } else {
        T &elem = buffer->produce();
        assert(elem.size() > 0);
        // Do not fill the buffer as this might cause deadlock, because
        // the last part of a time slice will never be released to the
        // buffer.
        int size = data_reader->get_bytes(elem.size(), elem.buffer());
        if (size < 0) {
          // Make sure the error messages do not propagate in the buffer
          size = 0;
        }
        buffer->produced(size);
      }
    }
  }
}


#endif // DATA_READER2BUFFER_H
