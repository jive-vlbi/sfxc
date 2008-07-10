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

#include "data_reader.h"
#include "utils.h"

#include <memory_pool.h>
#include <threadsafe_queue.h>

#include <pthread.h>
#include <boost/shared_ptr.hpp>

/** Reads data from the data reader and puts it in a buffer,
 * which is useful for non-blocking IO.
 **/
template <class T>
class Data_reader2buffer {
  typedef Data_reader2buffer<T> Self;
public:
  typedef T                                          data_type;
  typedef Memory_pool<data_type>                     Memory_pool;
  typedef typename Memory_pool::value_type           pool_type;
  struct value_type {
    int       actual_size;
    pool_type data;
  };
  typedef Threadsafe_queue<value_type>               Queue;
  typedef boost::shared_ptr<Queue>                   Queue_ptr;

  typedef boost::shared_ptr< Data_reader > Data_reader_ptr;

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

  Queue_ptr get_queue();
  void set_queue(Queue_ptr queue);

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
  Memory_pool     memory_pool;
  Queue_ptr       queue;
  State           state;
  pthread_t       io_thread;

  pthread_mutex_t mutex_for_set_state;
};


// Implementation:
template <class T>
Data_reader2buffer<T>::Data_reader2buffer()
    : memory_pool(250), state(STOPPED) {
  pthread_mutex_init(&mutex_for_set_state, NULL);
}

template <class T>
Data_reader2buffer<T>::
Data_reader2buffer(const Data_reader2buffer &buffer) {
  // No copy constructor, the threads don't like it
  SFXC_ASSERT_MSG(false,
                  "No copy constructor available for Data_reader2buffer");
}

template <class T>
Data_reader2buffer<T>::~Data_reader2buffer() {
  stop();
}

template <class T>
void
Data_reader2buffer<T>::set_data_reader(Data_reader_ptr reader) {
  SFXC_ASSERT(state != RUNNING);
  SFXC_ASSERT(data_reader == Data_reader_ptr());
  data_reader = reader;
}

template <class T>
typename Data_reader2buffer<T>::Data_reader_ptr
Data_reader2buffer<T>::get_data_reader() {
  return data_reader;
}

template <class T>
void
Data_reader2buffer<T>::set_queue(Queue_ptr queue_) {
  SFXC_ASSERT(queue == Queue_ptr());
  SFXC_ASSERT(state == STOPPED);
  queue = queue_;
}

template <class T>
typename Data_reader2buffer<T>::Queue_ptr
Data_reader2buffer<T>::get_queue() {
  return queue;
}


template <class T>
void
Data_reader2buffer<T>::try_start() {
  if ((data_reader != Data_reader_ptr() ) && (queue != Queue_ptr())) {
    start();
  }
}

template <class T>
void
Data_reader2buffer<T>::start() {
  SFXC_ASSERT(data_reader != Data_reader_ptr());
  SFXC_ASSERT(queue != Queue_ptr());

  DEBUG_MSG("Threaded reader !");
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
  if (state == STOPPED)
    return;
  set_state(STOPPED);
  pthread_join(io_thread, NULL);
  DEBUG_MSG("Threaded reader stopped !");
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
  DEBUG_MSG("Threaded reading !");
  self->read();
  return NULL;
}

template <class T>
void
Data_reader2buffer<T>::read() {
  while (state != STOPPED) {
    if (state == SUSPENDED) {
      usleep(1000); // 100 microseconds
    } else {
      if (memory_pool.empty()) {
        usleep(1000); // 100 microseconds
      } else if (data_reader->eof()) {
        DEBUG_MSG("data_reader->eof()");
        set_state(STOPPED);
      } else {
        if (data_reader->can_read()) {
          SFXC_ASSERT(!memory_pool.empty());
          value_type elem;
          elem.data = memory_pool.allocate();
          SFXC_ASSERT(elem.data->size() > 0);
          // Do not fill the buffer as this might cause deadlock, because
          // the last part of a time slice will never be released to the
          // buffer.
          int size = data_reader->get_bytes(elem.data->size(),
                                            elem.data->buffer());
          if (size > 0) {
            // Make sure the error messages do not propagate in the buffer
            // And that we do not insert empty buffers in the queue
            // The allocated elements are automatically released
            elem.actual_size = size;
            queue->push(elem);
          } else {
            // Couldn't read, sleep
            usleep(1000);
          }
        } else {
          usleep(1000);
        }
      }
    }
  }
}


#endif // DATA_READER2BUFFER_H
