/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */


//sfxc includes
#include "Integration_slice.h"

// Initialise the correlation for one integration slice
Integration_slice::Integration_slice(Log_writer &lg_wrtr)
  //member initialisations
  :dc(lg_wrtr), cc(), parameters_set(false), log_writer(lg_wrtr)
{
}


Integration_slice::Integration_slice(
  GenP &GenPrms, 
  StaP *StaPrms,
  Log_writer &lg_wrtr,
  int ref_station)
  //member initialisations
  :dc(lg_wrtr), cc(), parameters_set(false), log_writer(lg_wrtr)
{
  set_parameters(GenPrms,StaPrms, ref_station);
}

void
Integration_slice::set_parameters(
  GenP &GenPrms, 
  StaP *StaPrms,
  int  ref_station)
{
  // Only set the parameters once, otherwise the arrays get constructed twice
  assert( !parameters_set );
  parameters_set = true;
  
  dc.set_parameters(GenPrms, StaPrms);
  cc.set_parameters(GenPrms, ref_station);
  
  Nsegm2Avg = INT32 (2 * GenPrms.get_bwfl() / GenPrms.get_n2fft());
  Nsegm2Avg = (INT32) (GenPrms.get_time2avg() * Nsegm2Avg);
}





//pass the delay table
bool Integration_slice::set_delay_table(int i, DelayTable &delay_table)
{
  return dc.set_delay_table(i,delay_table);
}


//pass the data reader
void Integration_slice::set_data_reader(int sn, Data_reader *data_reader)
{
  dc.set_data_reader(sn,data_reader);
}


//pass the data writer 
void Integration_slice::set_data_writer(Data_writer *data_writer)
{
  cc.set_data_writer(data_writer);
}

void Integration_slice::set_start_time_and_duration(INT64 us_start, int duration) {
  dc.set_start_time_and_duration(us_start, duration);
}

//initialise reader to proper position
bool Integration_slice::init_reader(int sn, INT64 startIS)
{
  return dc.init_reader(sn,startIS);
}


// Correlates all the segments (Nsegm2Avg) in the integration slice.
bool Integration_slice::correlate()
{  
  bool result = true;

  float TenPct=Nsegm2Avg/10.0, i=0;
  log_writer(1) << "Nsegm2Avg " << Nsegm2Avg << endl;
  
  //zero accumulation accxps array and norms array.
  result = cc.init_time_slice();

  //process all the segments in the Time Slice (=Time to Average)
  for (INT32 segm = 0 ; result && (segm < Nsegm2Avg) ; segm++){

    //fill the current segment in cc with delay corrected data from dc
    result &= dc.fill_segment();
    //do the correlation for current segment.
    result &= cc.correlate_segment(dc.get_segment());

    if ( floor((segm+1)/TenPct) == i+1 ){
      i++;
      log_writer(1) << "segm=" << segm << "\t " << i*10 << 
      " % of current Integration Slice processed\n";
    }
  }

  if (!result) return false;
  
  //normalise the accumulated correlation results
  result = cc.average_time_slice();

  if (!result) return false;

  //write the correlation result for the current time slice
  result = cc.write_time_slice();

  return result;
}



// Correlates all the segments (Nsegm2Avg) in the integration slice.
bool Integration_slice::correlate(Timer &cd_tmr, Timer &cc_tmr, Timer &cx_tmr, Timer &dc_tmr)
{  
  bool result = true;

  float TenPct=Nsegm2Avg/10.0, i=0;
  log_writer(1) << "Nsegm2Avg " << Nsegm2Avg << endl;
  
  //zero accumulation accxps array and norms array.
  result = cc.init_time_slice();

  //process all the segments in the Time Slice (=Time to Average)
  for (INT32 segm = 0 ; result && (segm < Nsegm2Avg) ; segm++){

    cd_tmr.start();
    //fill the current segment in cc with delay corrected data from dc
    result &= dc.fill_segment(cx_tmr, dc_tmr);
    cd_tmr.stop_accumulate();

    cc_tmr.start();
    //do the correlation for current segment.
    result &= cc.correlate_segment(dc.get_segment());
    cc_tmr.stop_accumulate();

    if ( floor((segm+1)/TenPct) == i+1 ){
      i++;
      log_writer(1) << "segm=" << segm << "\t " << i*10 << 
      " % of current Integration Slice processed\n";
    }
  }

  if (!result) return false;
  
  cc_tmr.start();
  //normalise the accumulated correlation results
  result = cc.average_time_slice();

  if (!result) return false;

  //write the correlation result for the current time slice
  result = cc.write_time_slice();
  cc_tmr.stop_accumulate();

  return result;
}



Data_writer &Integration_slice::get_data_writer() {
  return cc.get_data_writer();
}


Log_writer& Integration_slice::get_log_writer()
{
  return log_writer;
}

