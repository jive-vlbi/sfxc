#ifndef TIME_SLICER_H
#define TIME_SLICER_H

#include <iostream>
#include <Buffer.h>
#include <Data_writer.h>

#include <boost/shared_ptr.hpp>
#include <map>
#include <semaphore.h>
#include <assert.h>

// A time slicer is a buffer with size elements and has access to at
// least backup_elements
template <class T = Buffer_element<char, 131072> >
class Time_slicer : Buffer<T> {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Base;
  typedef Time_slicer<T>                          Self;

  Time_slicer(int size, int backup_elements);
  ~Time_slicer();

  // Time_slicer specific functions
  void add(boost::shared_ptr<Data_writer> writer, int64_t start);
  bool do_task();
  bool finished();

  // Buffer specific functions
  T &produce();
  void produced(int status);

  // These should not be used, as the time slicer consumes the data itself
  T &consume(int &status);
  void consumed();

  bool empty();
  bool full();

private:
  /// Returns whether the slice is completely written
  void write(char *elem, int size, boost::shared_ptr<Data_writer> &writer);
  T &slicer_consume(int &status);
  void slicer_consumed();

  // Two semaphores to avoid overwriting of data
  sem_t empty_sem, full_sem;
  // For debugging purposes:
  bool consuming, producing;
  int backup_elements;
  
  int64_t data_counter;

  // Output streams
  typedef std::multimap<int, boost::shared_ptr<Data_writer> > Writers;
  typedef typename Writers::iterator                          Writer_iterator;
  Writers                                                     writers;
};



///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////

template <class T>
Time_slicer<T>::
Time_slicer(int size, int backup_elements) 
  : Base(size), consuming(false), producing(false), 
    backup_elements(backup_elements),
    data_counter(0)
{
  assert(size > 0);
  if ( sem_init(&empty_sem, 1, 0) == -1 ) {
    std::cout << "Failed to initialise the \"empty\" semaphore" << std::endl;
    exit(1);
  }
  if ( sem_init(&full_sem, 1, size-backup_elements) == -1 ) {
    std::cout << "Failed to initialise the \"full\" semaphore" << std::endl;
    exit(1);
  }
}

template <class T>
Time_slicer<T>::~Time_slicer() {
}

template <class T>
void
Time_slicer<T>::
add(boost::shared_ptr<Data_writer> writer, int64_t start) {
  // Write the data that has already been sent to the other writers
  if (start < data_counter) {
    int to_read = data_counter-start;
    int index=0;
    // Go back in the buffer until we found the right element:
    while (to_read > 0) {
      index ++;
      assert(index <= backup_elements);
      int size;
      Base::get_cons_elem_prev(size, index);
      to_read -= size;
    }
    { // First buffer:
      int size;
      T &elem = Base::get_cons_elem_prev(size, index);
      // to_read is negative:
      write(&elem[-to_read], size+to_read, writer);
      index--;
    }
    for (; index>0; index--) { // process all other buffers
      int size;
      T &elem = Base::get_cons_elem_prev(size, index);
      write(elem.buffer(), size, writer);
    }
  }
  if (writer->get_size_dataslice()==0) return;
  writers.insert(std::make_pair(start, writer));
}

template <class T>
bool Time_slicer<T>::do_task() {
  if (!empty()) {
    if (writers.empty()) {
      DEBUG_MSG("No output stream available");
      return false;
    } else if (writers.begin()->first > data_counter) {
      DEBUG_MSG("Output starts after current time");
      return false;
    } else {
      int size;
      T &elem = slicer_consume(size);
      for (Writer_iterator it = writers.begin(); 
	   (it != writers.end()) && (it->first <= data_counter);
	   it++) {
	write(elem.buffer(), size, it->second);
      }
      while ((!writers.empty()) && 
	     (writers.begin()->second->get_size_dataslice()==0)) {
        DEBUG_MSG("Writers.erase(begin);");
	writers.erase(writers.begin());
      }
      data_counter += size;
      slicer_consumed();
    }
    return true;
  } else {
    DEBUG_MSG("Buffer empty");
    return false;
  }
}

template <class T>
bool
Time_slicer<T>::finished() {
  return writers.empty();
}

template <class T>
T &
Time_slicer<T>::produce() {
  assert(!producing); producing = true;
  int val;
  sem_getvalue(&full_sem, &val);

  sem_wait(&full_sem);

  return Base::get_prod_elem();
}

template <class T>
void
Time_slicer<T>::produced(int status) {
  assert(producing); producing = false;
  Base::succ_prod(status);
  sem_post(&empty_sem);
}
  
template <class T>
T &
Time_slicer<T>::consume(int &status) {
  assert(false);
}

template <class T>
void
Time_slicer<T>::consumed() {
  assert(false);
}

template <class T>
bool 
Time_slicer<T>::empty() {
  int val;
  sem_getvalue(&empty_sem, &val);
  return (val <= 0);
}

template <class T>
bool 
Time_slicer<T>::full() {
  int val;
  sem_getvalue(&full_sem, &val);
  return (val == 0);
}

template <class T>
void
Time_slicer<T>::
write(char *buffer, int size, boost::shared_ptr<Data_writer> &writer) {
  int slice_size = writer->get_size_dataslice();
  if ((slice_size == -1) || (slice_size > size)) {
    writer->put_bytes(size, buffer);
  } else {
    writer->put_bytes(slice_size, buffer);
  }
}

template <class T>
T &
Time_slicer<T>::slicer_consume(int &status) {
  assert(!consuming); consuming = true;
  sem_wait(&empty_sem);
  return Base::get_cons_elem(status);
}

template <class T>
void
Time_slicer<T>::slicer_consumed() {
  int val;
  sem_getvalue(&full_sem, &val);

  assert(consuming); consuming = false;
  Base::succ_cons();
  sem_post(&full_sem);
}

#endif // TIME_SLICER_H
