/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 *  This file contains:
 *     -the definition of the data_reader_factory object.
 */
#include "exception_common.h"

#include "data_reader_factory.h"
#include "data_reader_file.h"
#include "data_reader_mk5.h"

Data_reader* Data_reader_factory::get_reader(const std::vector<std::string>& sources) {
  if (sources[0].find("file://") == 0)
    return new Data_reader_file(sources);
  if (sources[0].find("mk5://") == 0)
    return new Data_reader_mk5(sources[0]);

  MTHROW("No data reader to handle :" + sources[0]);
}
