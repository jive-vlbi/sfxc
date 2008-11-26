#include <arpa/inet.h>
#include <unistd.h>
#include <sched.h>
#include "data_reader_blocking.h"
#include "utils.h"

Data_reader_blocking::Data_reader_blocking(Data_reader *rdr) {
  m_reader = rdr;
  SFXC_ASSERT(rdr);
}

size_t Data_reader_blocking::get_bytes_s(Data_reader* reader, size_t size, char* buffer)
{
  int numretry = 0;
  size_t remains = size;

  /// Loop until the file is ended or that there is more requested byte
  /// unsatisfiad.
  while ( !reader->eof() && remains != 0 ) {
    int read = reader->get_bytes(remains, (buffer == NULL)?NULL: buffer + (size-remains) );
    if ( read > 0 ) {
      remains -= read;
    } else if ( read == 0 ) {
      /// Check if the file is closed and that this would explain that
      /// it is not possible to read data anymore. (this may be possible with
      /// if the data_reader is a socket.
      if ( reader->eof() ) return size-remains;
    } else {
      /// Return the error value
      return read;
    }
  }
  return size-remains;
}


size_t Data_reader_blocking::get_bytes_s(Data_reader* reader, size_t size, char* buffer, int max_retry)
{
  int numretry = 0;
  size_t remains = size;

  /// Loop until the file is ended or that there is more requested byte
  /// unsatisfiad.
  while ( !reader->eof() && remains != 0 ) {
    int read = reader->get_bytes(remains, (buffer == NULL)?NULL: buffer + (size-remains) );
    if ( read >= 0 ) {
      remains -= read;

      /// For each incomplete read request we increase a counter
      numretry++;
      if ( numretry >= max_retry) {
        numretry = max_retry;
      }

      /// We may even sleep a bit in case the system is really slow.
      if( remains != 0 )
      {
      	if( numretry <= 10 ) usleep(1000);
				else sched_yield();
      }
    } else if ( read == 0 && numretry == max_retry ) {
      /// Check if the file is closed and that this would explain that
      /// it is not possible to read data anymore. (this may be possible with
      /// if the data_reader is a socket.
      if ( reader->eof() ) return size-remains;
    } else {
      /// Return the error value
      return read;
    }
  }
  return size-remains;

}


size_t Data_reader_blocking::do_get_bytes(size_t size, char* buffer) {
  int numretry = 0;
  size_t remains = size;

  /// Loop until the file is ended or that there is more requested byte
  /// unsatisfiad.
  while ( !eof() && remains != 0 ) {
    int read = m_reader->get_bytes(remains, buffer+ (size-remains) );
    if ( read >= 0 ) {
      remains -= read;

      /// For each incomplete read request we increase a counter
      numretry++;
      if ( numretry >= 100 ) {
        numretry = 100;
      }

      /// We may even sleep a bit in case the system is really slow.
			if( remains != 0 )
      {
      	if( numretry <= 10 ) usleep(100);
				else sched_yield();
      }
    } else if ( read == 0 && numretry == 100 ) {
      /// Check if the file is closed and that this would explain that
      /// it is not possible to read data anymore. (this may be possible with
      /// if the data_reader is a socket.
      if ( eof() ) return size-remains;
    } else {
      /// Return the error value
      return read;
    }
  }
  return size-remains;
}

bool Data_reader_blocking::eof() {
  return m_reader->eof();
}

bool Data_reader_blocking::can_read() {
  DEBUG_MSG("Data_reader_blocking: can read not implemented");
  return true;
}

Data_reader_blocking& operator>>(Data_reader_blocking& dr, uint32_t& value) {
  dr.get_bytes(sizeof(uint32_t), (char*)&value);
  value = ntohl(value);
  return dr;
}


Data_reader_blocking& operator>>(Data_reader_blocking& dr, std::string& str) {
  uint32_t size;
  dr.get_bytes(sizeof(uint32_t), (char*)&size);
  char tmp[size+1];
  dr.get_bytes(size+1, tmp );
  str = tmp;
  return dr;
}

Data_reader_blocking& operator>>(Data_reader_blocking& dr, int32_t& value) {
  dr.get_bytes(sizeof(int32_t), (char*)&value);
  value = ntohl(value);
  return dr;
}
