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
#include "data_reader_dnfp.h"
#include "data_reader_file.h"
#include "data_reader_mark5.h"

Data_reader* Data_reader_factory::get_reader(const std::string& url) {
  if ( url.find("file://") == 0 ) {
    return new Data_reader_file(url);
  } else if ( url.find("dnfp://") == 0 ) {
    return new Data_reader_dnfp(url);
  } else if ( url.find("mark5://") == 0 ) {
    return new Data_reader_mark5(url);
  }

  MTHROW( "No data reader to handle :"+url );
}

