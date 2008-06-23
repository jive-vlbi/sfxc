/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef BUFFER2DATA_WRITER_H
#define BUFFER2DATA_WRITER_H

#include "data_writer.h"
#include "utils.h"

#include <boost/shared_ptr.hpp>

#include <threadsafe_queue.h>

/** Reads data from a buffer and writes it to a Data_writer.
 **/
template <class T>
class Buffer2data_writer {
  typedef Buffer2data_writer<T> Self;

public:
  typedef boost::shared_ptr< Data_writer > Data_writer_ptr;
  typedef Threadsafe_queue<T>              Queue;
  typedef boost::shared_ptr<Queue>         Queue_ptr;
  
  enum State {
    STOPPED=0, ///< Not running, the additional thread is not active
    SUSPENDED, /**< Not running, the additional thread is waiting
                            (e.g. for a change of buffers) **/
    RUNNING    ///< The thread is writing data from the buffer
  };

  Buffer2data_writer();
  ~Buffer2data_writer();

  void set_data_writer(Data_writer_ptr data_writer);
  Data_writer_ptr get_data_writer();
  void set_queue(Queue_ptr queue_);
  Queue_ptr get_queue();

  /// Starts the asynchronous IO if the buffer and data_writer are not NULL
  void try_start();
  /** Starts the asynchronous IO.
   * \pre the buffer and the data_writer are not NULL
   **/
  void start();
  /// Stop the asynchronous IO, blocks until stopped.
  void stop();
  /// Suspend the asynchronous IO, do not stop the thread
  void suspend();
  /// Resume the asynchronous IO, the thread is already created
  void resume();

  /// Return the status
  State status();

private:
  // Functions for the thread:
  static void *start_writing(void *);
  void write();

  boost::shared_ptr< Data_writer >         data_writer;
  boost::shared_ptr< Threadsafe_queue<T> > queue;
  State       state;
  pthread_t   redirect_thread;
};


// Implementation:
template <class T>
Buffer2data_writer<T>::Buffer2data_writer()
    : state(STOPPED), redirect_thread(0) {}

template <class T>
Buffer2data_writer<T>::~Buffer2data_writer() {
  if (queue != Queue_ptr()) {
    if (!queue->empty()) {
      std::cout << "Buffer of Buffer2data_writer is not empty" << std::endl;
    }
  }
  stop();
}

template <class T>
void
Buffer2data_writer<T>::set_data_writer(boost::shared_ptr< Data_writer > writer) {
  SFXC_ASSERT(state != RUNNING);
  data_writer = writer;
}

template <class T>
void
Buffer2data_writer<T>::set_queue(Queue_ptr queue_) {
  SFXC_ASSERT(state != RUNNING);
  queue = queue_;
}

template <class T>
boost::shared_ptr< Data_writer >
Buffer2data_writer<T>::get_data_writer() {
  return data_writer;
}

template <class T>
typename Buffer2data_writer<T>::Queue_ptr
Buffer2data_writer<T>::get_queue() {
  return queue;
}

template <class T>
void
Buffer2data_writer<T>::try_start() {
  if ((data_writer != NULL) && (queue != Queue_ptr())) {
    start();
  }
}

template <class T>
void
Buffer2data_writer<T>::start() {
  SFXC_ASSERT(data_writer != NULL);
  SFXC_ASSERT(queue != Queue_ptr());
  SFXC_ASSERT(state == STOPPED);

  state = RUNNING;
  pthread_create(&redirect_thread, NULL,
                 start_writing, static_cast<void*>(this));
}

template <class T>
void
Buffer2data_writer<T>::stop() {
  if (state == STOPPED) return;
  state = STOPPED;
  pthread_join(redirect_thread, NULL);
}

template <class T>
typename Buffer2data_writer<T>::State
Buffer2data_writer<T>::status() {
  return state;
}

template <class T>
void *
Buffer2data_writer<T>::start_writing(void * self_) {
  Self *self = static_cast<Self *>(self_);
  self->write();
  return NULL;
}

template <class T>
void
Buffer2data_writer<T>::write() {
  // Always empty the queue
  while (!((state == STOPPED) && queue->empty())) {
    if ((state == SUSPENDED) || queue->empty()) {
      usleep(100000); // .1 second:
    } else {
      T &elem = queue->front();

      int size = elem.actual_size;
      int size2 = 0;
      char *buff = elem.data->buffer();
      while (size != size2) {
        int64_t new_size = data_writer->put_bytes(size,buff);
        if (new_size <= 0) {
          DEBUG_MSG("Error writing data");
          usleep(100000); // .1 second:
        } else {
          size2 += new_size;
          buff += new_size;
        }
      }
      SFXC_ASSERT((state != RUNNING) || (size == size2));
      queue->pop();
    }
  }
}


#endif // BUFFER2DATA_WRITER_H
