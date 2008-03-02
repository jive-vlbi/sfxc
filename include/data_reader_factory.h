#ifndef DATA_READER_DFACTORY_H
#define DATA_READER_DFACTORY_H

#include <string>
#include "data_reader.h"

class Data_reader_factory
{
public:
  static Data_reader* get_reader(const std::string& url);
};

#endif // DATA_READER_FACTORY_H
