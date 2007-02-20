#ifndef DATA_READER2BUFFER_H
#define DATA_READER2BUFFER_H

#include <Data_reader.h>
#include <Buffer.h>

#include <assert.h>

/** Reads data from the data reader and puts it in a buffer,
 * which is useful for non-blocking IO.
 **/
template <class T>
class Data_reader2buffer
{
  typedef Data_reader2buffer<T> Self;
public:
  enum State {
    STOPPED, ///< Not running, the additional thread is not active
    SUSPENDED, /**< Not running, the additional thread is waiting 
                    (e.g. for a change of buffers) **/
    RUNNING    ///< The thread is writing data from the buffer
  };

  Data_reader2buffer();
  ~Data_reader2buffer();
  
  Data_reader *get_data_reader();
  void set_data_reader(Data_reader *data_reader);

  Buffer<T> *get_buffer();
  void set_buffer(Buffer<T> *buffer);
  
  void start();
  void try_start();
  void stop();
  State get_state();

private:
  static void *start_reading(void *);
  void read();
private:
  Data_reader *data_reader;
  Buffer<T>   *buffer;
  State       state;
  pthread_t   redirect_thread;
};


// Implementation:
template <class T>
Data_reader2buffer<T>::Data_reader2buffer() 
  : data_reader(NULL), buffer(NULL), state(STOPPED)
{
}

template <class T>
Data_reader2buffer<T>::~Data_reader2buffer() 
{
  stop();
}
  
template <class T>
void
Data_reader2buffer<T>::set_data_reader(Data_reader *reader) {
  assert(state != RUNNING);
  data_reader = reader;
}

template <class T>
Data_reader *
Data_reader2buffer<T>::get_data_reader() {
  return data_reader;
}

template <class T>
void
Data_reader2buffer<T>::set_buffer(Buffer<T> *buff) {
  assert(state != RUNNING);
  buffer = buff;
}

template <class T>
Buffer<T> *
Data_reader2buffer<T>::get_buffer() {
  return buffer;
}

  
template <class T>
void
Data_reader2buffer<T>::try_start() {
  if ((data_reader != NULL) && (buffer != NULL)) {
    start();
  }
}

template <class T>
void
Data_reader2buffer<T>::start() {
  assert(data_reader != NULL);
  assert(buffer != NULL);
  
  state = RUNNING;
  pthread_create(&redirect_thread, NULL, 
                 start_reading, static_cast<void*>(this));
}

template <class T>
void
Data_reader2buffer<T>::stop() {
  if (state == STOPPED) return;
  state = STOPPED;
  pthread_join(redirect_thread, NULL);
}

template <class T>
typename Data_reader2buffer<T>::State
Data_reader2buffer<T>::get_state() {
  return state;
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
    if ((state == SUSPENDED) || buffer->full() || data_reader->eof()) {
      usleep(100000); // .1 second:
    } else {
      T &elem = buffer->produce();
      int size = data_reader->get_bytes(sizeof(T),(char*)&elem);
      buffer->produced(size);
      if (size == 0) state = SUSPENDED;
    }
  }
}


#endif // DATA_READER2BUFFER_H
