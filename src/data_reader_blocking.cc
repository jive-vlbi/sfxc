#include <arpa/inet.h>
# include <unistd.h>
#include "data_reader_blocking.h"

Data_reader_blocking::Data_reader_blocking(Data_reader *rdr)
{
    m_reader = rdr;
    assert(rdr);
}

int Data_reader_blocking::do_get_bytes(size_t size, char* buffer)
{
    int numretry = 0;
    size_t remains = size;
    while ( !eof() && remains != 0 )
    {
        size_t read = m_reader->get_bytes(remains, buffer+ (size-remains) );
        remains -= read;
        numretry++;
        if ( numretry >= 100 )
        {
            numretry = 100;
        }
        if ( remains != 0 ) usleep(numretry*100);
    }
    return remains;
}

bool Data_reader_blocking::eof()
{
    return m_reader->eof();
}

Data_reader_blocking& operator>>(Data_reader_blocking& dr, uint32_t& value)
{
    dr.get_bytes(sizeof(uint32_t), (char*)&value);
    value = ntohl(value);
    return dr;
}


Data_reader_blocking& operator>>(Data_reader_blocking& dr, std::string& str)
{
    uint32_t size;
    dr.get_bytes(sizeof(uint32_t), (char*)&size);
    char tmp[size+1];
    dr.get_bytes(size+1, tmp );
    str = tmp;
    return dr;
}

Data_reader_blocking& operator>>(Data_reader_blocking& dr, int32_t& value)
{
    dr.get_bytes(sizeof(int32_t), (char*)&value);
    value = ntohl(value);
    return dr;
}
