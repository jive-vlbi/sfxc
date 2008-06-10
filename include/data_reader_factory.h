/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 */
#ifndef DATA_READER_DFACTORY_H
#define DATA_READER_DFACTORY_H

#include <string>
#include "data_reader.h"

class Data_reader_factory {
public:
  static Data_reader* get_reader(const std::string& url);
};

#endif // DATA_READER_FACTORY_H
