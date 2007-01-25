#ifndef DATA_WRITER_FILE_H
#define DATA_WRITER_FILE_H

#include <Data_writer.h>

#include <vector>

class Data_writer_file : public Data_writer {
public:
  Data_writer_file(const char *filename);
  ~Data_writer_file();
  
  UINT64 put_bytes(UINT64 nBytes, char *buff);

private:
  FILE *file;
};

#endif // DATA_WRITER_FILE_H
