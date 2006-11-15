/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$
*/

#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <types.h>

/** Virtual class defining the interface for obtaining input.
 **/
class Input_reader {
  public:
  /** Moves the read_pointer nBytes bytes forward. 
      @param[in] nBytes Number of bytes to advance.
      \pre nBytes >= 0
      \return Returns the number of bytes advanced.
  **/
  virtual INT64 move_forward(INT64 nBytes) = 0;

  /** Reads nBytes from channel into the buff starting from the
      read_pointer.
      \return the number of bytes read into the buffer.
      \pre buff is allocated. nBytes >= 0
  **/ 
  virtual INT64 get_bytes(INT64 nBytes, char *buff) = 0;

//   /** Returns the position of the next header, or -1 if no header is
//       found on the input.
//   **/
//   virtual int header_position() = 0;

//   /** Returns the next header. 
//       \pre There is another header on the input.
//   **/
//   virtual Mk4header header() = 0;
};

#endif // INPUT_READER_H
