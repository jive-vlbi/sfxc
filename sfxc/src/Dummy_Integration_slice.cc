/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Dummy implementation of Integration_slice for testing the data flow of the 
 * multicore version without actually performing the correlation.
 */

//sfxc includes
#include "Integration_slice.h"

// "Local" global variables
Log_writer  *int_slice_log_writer;
INT64        int_slice_startTS;

// Initialise the correlation for one integration slice
Integration_slice::Integration_slice(
  Log_writer &lg_wrtr)
  //member initialisations
  :dc(lg_wrtr), cc()
{
  int_slice_log_writer = &lg_wrtr;
  int_slice_startTS = 0;
}

Integration_slice::Integration_slice(
  GenP &GenPrms, 
  StaP* StaPrms,
  Log_writer &lg_wrtr)
  //member initialisations
  :dc(lg_wrtr), cc()
{
  int_slice_log_writer = &lg_wrtr;
  int_slice_startTS = 0;
}

void Integration_slice::set_parameters(GenP &GenPrms, StaP* StaPrms) {
} 

//pass the delay table
void Integration_slice::set_delay_table(int i, DelayTable &delay_table)
{
}


//pass the data reader
void Integration_slice::set_data_reader(int sn, Data_reader *data_reader)
{
//  dc.set_data_reader(sn,data_reader);
}


//pass the data writer 
void Integration_slice::set_data_writer(Data_writer *data_writer)
{
  cc.set_data_writer(data_writer);
}


//initialise reader to proper position
void Integration_slice::init_reader(int sn, INT64 startIS)
{
}


// Correlates all the segments (Nsegm2Avg) in the integration slice.
void Integration_slice::correlate()
{  
  char data[80];
  sprintf(data, "%ld\n", int_slice_startTS);
  int_slice_startTS++;
  get_data_writer().put_bytes(strlen(data), data);
  usleep(100000);
  
}

Data_writer &Integration_slice::get_data_writer() {
  return cc.get_data_writer();
}

