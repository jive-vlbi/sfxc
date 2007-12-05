#include <iostream>
#include <boost/shared_ptr.hpp>

#include "time_slicer.h"
#include "data_writer_file.h"


typedef Buffer_element<char,4> Element;
typedef Time_slicer< Element > Slicer;

void produce(Slicer &slicer) {
  static int count =0;
  if (slicer.full()) {
    std::cout << "Slicer buffer is full" << std::endl;
    return;
  }
  Element &elem=slicer.produce();
  snprintf(elem.buffer(), 4, "%2d\n", count++);
  slicer.produced(4);
}

int main() {
  int elements          = 5;
  int buffered_elements = 4;
  Slicer slicer(elements, buffered_elements);

  // Initialise the buffer
  for (int i=0; i<elements-buffered_elements; i++) {
    produce(slicer);
  }

  boost::shared_ptr<Data_writer> out(new Data_writer_file("file://1.txt"));
  out->set_size_dataslice(60);
  slicer.add(out, /* start byte */0);

  for (int i=0; i<buffered_elements; i++) {
    slicer.do_task();
    produce(slicer);
  }

  boost::shared_ptr<Data_writer> out2(new Data_writer_file("file://2.txt"));
  out2->set_size_dataslice(12);
  slicer.add(out2, /* start byte */0);

  while (! slicer.finished()) {
    slicer.do_task();
    produce(slicer);
  }
}
