/*
  CVS keywords
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef DATA_READER_CONTROLLER_H
#define DATA_READER_CONTROLLER_H

#include <types.h>
#include <Controller.h>

/** Virtual class defining the interface for obtaining input.
 **/
class Data_reader_controller : public Controller {
public:
  Data_reader_controller(Data_reader &);

  class Producer {
  public:
    Producer(Data_reader &reader);
    int operator()(T &);
  };

  int process_event(const MPI_Status &status);

  Producer get_producer();
private:
  Data_reader reader;
};

#endif // DATA_READER_CONTROLLER_H
