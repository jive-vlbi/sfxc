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

#include <Data_writer.h>
#include <Buffer.h>


#include <assert.h>

/** Reads data from a buffer and writes it to a Data_writer.
 **/
template <class T>
class Buffer2data_writer
{
  typedef Buffer2data_writer<T> Self;
  
public:
  enum State {
    STOPPED, ///< Not running, the additional thread is not active
    SUSPENDED, /**< Not running, the additional thread is waiting 
                    (e.g. for a change of buffers) **/
    RUNNING    ///< The thread is writing data from the buffer
  };

  Buffer2data_writer();
  ~Buffer2data_writer();
  
  void set_data_writer(Data_writer *data_writer);
  void set_buffer(Buffer<T> *buffer);
  Data_writer *get_data_writer();
  Buffer<T>   *get_buffer();

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

  Data_writer *data_writer;
  Buffer<T>   *buffer;
  State       state;
  pthread_t   redirect_thread;
};


// Implementation:
template <class T>
Buffer2data_writer<T>::Buffer2data_writer() 
  : data_writer(NULL), buffer(NULL), state(STOPPED), redirect_thread(0)
{
}

template <class T>
Buffer2data_writer<T>::~Buffer2data_writer() 
{
  if (buffer != NULL) {
    if (!buffer->empty()) {
      std::cout << "Buffer of Buffer2data_writer is not empty" << std::endl;
    }
  }
  stop();
}
  
template <class T>
void
Buffer2data_writer<T>::set_data_writer(Data_writer *writer) {
  assert(state != RUNNING);
  data_writer = writer;
}

template <class T>
void
Buffer2data_writer<T>::set_buffer(Buffer<T> *buff) {
  assert(state != RUNNING);
  buffer = buff;
}
  
template <class T>
Data_writer *
Buffer2data_writer<T>::get_data_writer() {
  return data_writer;
}

template <class T>
Buffer<T>   *
Buffer2data_writer<T>::get_buffer() {
  return buffer;
}
  
template <class T>
void
Buffer2data_writer<T>::try_start() {
  if ((data_writer != NULL) && (buffer != NULL)) {
    start();
  }
}

template <class T>
void
Buffer2data_writer<T>::start() {
  assert(data_writer != NULL);
  assert(buffer != NULL);
  assert(state == STOPPED);
  
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
  while (state != STOPPED) {
    if ((state == SUSPENDED) || buffer->empty()){
      usleep(100000); // .1 second:
    } else {
      int size;
      T &elem = buffer->consume(size);
      INT64 size2 = data_writer->put_bytes(size,elem.buffer());
      if (size2 <= 0) {
        std::cout << "Error writing data, going to suspended state" << std::endl;
        state = SUSPENDED;
      }
      char *buff = elem.buffer() + size2;
      while ((state == RUNNING) && (size > size2)) {
        assert(size2 > 0);
        UINT64 new_size = size - size2; 
        new_size = data_writer->put_bytes(new_size,buff);
        if (new_size <= 0) {
          std::cout << "Error writing data, going to suspended state" << std::endl;
          state = SUSPENDED;
        } else {
          size2 += new_size;
          buff += new_size;
        }
      }        
      assert((state != RUNNING) || (size == size2));
      buffer->consumed();
    }
  }
}


#endif // BUFFER2DATA_WRITER_H
