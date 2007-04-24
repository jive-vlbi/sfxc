/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef INDATA_H
#define INDATA_H

#include <Data_reader.h>
#include <vector>
#include <Log_writer.h>

#include <staPrms.h>
#include <genPrms.h>
#include <runPrms.h>

void set_log_writer(Log_writer &log_writer_);

Log_writer &get_log_writer();
                
void show_MK4_header(Data_reader *data_reader, INT64 startIS, 
  StaP &StaPrms, GenP &GenPrms);

int  fill_Mk4frame(int sn, Data_reader &reader, double **Mk4frame, 
  StaP &StaPrms);

int  FindHeaderMk4(Data_reader &reader, int& jsynch,
  INT64& usTime, INT64 usStart, StaP &StaPrms, GenP &GenPrms);

#endif // INDATA_H
