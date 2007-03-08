#ifndef DATA_READER_H
#define DATA_READER_H

/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/


#include <types.h>
#include <iostream>

/** Virtual class defining the interface for obtaining input.
 **/
class Data_reader {
public:
  virtual ~Data_reader() {
  }

  /** Reads nBytes from channel into the buff starting from the
      read_pointer. If buff == NULL, then the buffer is not 
      filled and the amount of bytes is only read.
      
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/ 
  virtual UINT64 get_bytes(UINT64 nBytes, char *buff) = 0;
};


#endif // DATA_READER_H
