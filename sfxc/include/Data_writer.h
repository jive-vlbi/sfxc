#ifndef DATA_WRITER_H
#define DATA_WRITER_H

#include <types.h>

class Data_writer {
public:
  Data_writer() {}
  
  virtual ~Data_writer() {}
  
  /** Reads nBytes from channel into the buff starting from the
      read_pointer.
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/ 
  virtual UINT64 put_bytes(UINT64 nBytes, char *buff) = 0;
};

#endif // DATA_WRITER_H
