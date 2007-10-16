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
#include "utils.h"

// Initialise the correlation for one integration slice
Integration_slice::Integration_slice(Log_writer &lg_wrtr)
  //member initialisations
  :dc(lg_wrtr), cc(lg_wrtr), parameters_set(false), log_writer(lg_wrtr)
{
}


Integration_slice::Integration_slice(Correlation_parameters &corr_param, 
                                     Log_writer &lg_wrtr,
                                     int ref_station1,
                                     int ref_station2)
  : dc(lg_wrtr), cc(lg_wrtr), parameters_set(false), log_writer(lg_wrtr)
{
  set_parameters(corr_param, ref_station1, ref_station2);
}

void
Integration_slice::set_parameters(
  Correlation_parameters &corr_param, 
  int ref_station1, int ref_station2)
{
  // Only set the parameters once, otherwise the arrays get constructed twice
  assert( !parameters_set );
  parameters_set = true;
  
  dc.set_parameters(corr_param);
  cc.set_parameters(corr_param, ref_station1, ref_station2);
  
  Nsegm2Avg = 
    (corr_param.sample_rate / 1000) * corr_param.integration_time
    / corr_param.number_channels;
  
  int bytes_in_integration = 
    // Offset for delay
    ((int64_t)(MAX_DELAY) * corr_param.sample_rate * corr_param.bits_per_sample) / 8000 +
    // bytes per seconds
    Nsegm2Avg * corr_param.number_channels * corr_param.bits_per_sample / 8;
}





//pass the delay table
bool Integration_slice::set_delay_table(int i, Delay_table_akima &delay_table)
{
  return dc.set_delay_table(i,delay_table);
}


//pass the data reader
void Integration_slice::set_sample_reader
  (int sn, 
   boost::shared_ptr<Bits_to_float_converter> sample_reader)
{
  dc.set_sample_reader(sn, sample_reader);
}


//pass the data writer 
void Integration_slice::set_data_writer
  (boost::shared_ptr<Data_writer> data_writer)
{
  cc.set_data_writer(data_writer);
}

void Integration_slice::set_start_time(int64_t us_start) {
  dc.set_start_time(us_start);
}

//initialise reader to proper position
bool Integration_slice::init_reader(int sn, int64_t startIS)
{
  return dc.init_reader(sn,startIS);
}


// Correlates all the segments (Nsegm2Avg) in the integration slice.
bool Integration_slice::correlate()
{  
  bool result = true;

  int TenPct=Nsegm2Avg/10;
  log_writer(2) << "Nsegm2Avg " << Nsegm2Avg << endl;
  //zero accumulation accxps array and norms array.
  result = cc.init_time_slice();

  //process all the segments in the Time Slice (=Time to Average)
  for (int32_t segm = 0 ; result && (segm < Nsegm2Avg) ; segm++){
    //fill the current segment in cc with delay corrected data from dc
    result &= dc.fill_segment();
    //do the correlation for current segment.
    result &= cc.correlate_segment(dc.get_segment());

    if ( segm%TenPct == 0 ){
      log_writer(1) << "segm=" << segm << "\t " << segm*100/Nsegm2Avg 
                    << " % of current Integration Slice processed"
                    << std::endl;
//      DEBUG_MSG("segm=" << segm << "\t " << segm*100/Nsegm2Avg 
//                << " % of current Integration Slice processed");
    }
  }

  if (!result) {
    log_writer(0) << "Error in computing the integration" << std::endl;
    return false;
  }

  //normalise the accumulated correlation results
  result = cc.average_time_slice();

  if (!result) {
    log_writer(0) << "Error in averaging the integration" << std::endl;
    return false;
  }

  //write the correlation result for the current time slice
  result = cc.write_time_slice();
  return result;
}





Data_writer &Integration_slice::get_data_writer() {
  return cc.get_data_writer();
}


Log_writer& Integration_slice::get_log_writer()
{
  return log_writer;
}

