
#ifndef DATA_READER_BLOCKING_H_INCLUDED
#define DATA_READER_BLOCKING_H_INCLUDED

#include <cassert>
#include <unistd.h>
#include "data_reader.h"

class Data_reader_blocking : public Data_reader
{
  Data_reader* m_reader;
  public:
    Data_reader_blocking(Data_reader *rdr);

    int do_get_bytes(size_t size, char* buffer);
    bool eof();
};


Data_reader_blocking& operator>>(Data_reader_blocking& dr, std::string& str);
Data_reader_blocking& operator>>(Data_reader_blocking& dr, uint32_t& value);
Data_reader_blocking& operator>>(Data_reader_blocking& dr, int32_t& value);

#endif // DATA_READER_BLOCKING_H_INCLUDED
