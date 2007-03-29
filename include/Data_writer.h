#ifndef DATA_WRITER_H
#define DATA_WRITER_H

#include <types.h>

class Data_writer {
public:
  Data_writer();
  
  virtual ~Data_writer();
  
  /** Writes nBytes from buff to the output device.
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/ 
  UINT64 put_bytes(UINT64 nBytes, char *buff);
  
  /** Returns the number of bytes written
   **/
  UINT64 data_counter();

  /** Resets the number of bytes written
   **/
  void reset_data_counter();
  
private:
  /** Function that actually writes the data to the output device.
  **/ 
  virtual UINT64 do_put_bytes(UINT64 nBytes, char *buff) = 0;
  
  UINT64 _data_counter;
};

#endif // DATA_WRITER_H
