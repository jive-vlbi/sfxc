/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: sfxc_adam.cc 174 2007-03-20 09:08:42Z kruithof $
 * 
 * Dummy implementation of Integration_slice for testing the data flow of the 
 * multicore version without actually performing the correlation.
 */

/*
Author     : RHJ Oerlemans
StartDate  : 20070201
Last change: 20070209
*/


//sfxc includes
#include "Integration_slice.h"

// Initialise the correlation for one integration slice
Integration_slice::Integration_slice(
  GenP &GenPrms, 
  StaP* StaPrms,
  Log_writer &lg_wrtr)
  //member initialisations
  :dc(GenPrms, StaPrms, lg_wrtr), cc(GenPrms)
{
}



//pass the delay table
void Integration_slice::set_delay_table(int i, DelayTable &delay_table)
{
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


//initialise reader to proper position
void Integration_slice::init_reader(int sn, StaP &StaPrms, INT64 startIS)
{
}


// Correlates all the segments (Nsegm2Avg) in the integration slice.
void Integration_slice::correlate()
{  
  //TODO RHJO test/debug code
  float TenPct=Nsegm2Avg/10.0, i=0;
  cout << "Nsegm2Avg " << Nsegm2Avg << endl;
  
  //zero accumulation accxps array and norms array.
  cc.init_time_slice();

  //process all the segments in the Time Slice (=Time to Average)
  for (INT32 segm = 0 ; segm < Nsegm2Avg ; segm++){

    //fill the current segment in cc with delay corrected data from dc
    dc.fill_segment();
    //do the correlation for current segment.
    cc.correlate_segment(dc.get_segment());

    //TODO RHJO test/debug code
    if ( floor((segm+1)/TenPct) == i+1 ){
      i++;
      cout << "segm=" << setw(8) << segm << " " <<
      setw(3) << i*10 << " % of current Integration Slice processed\n";
    }
      
  }
  
  //normalise the accumulated correlation results
  cc.average_time_slice();

  //write the correlation result for the current time slice
  cc.write_time_slice();


}

Data_writer &Integration_slice::get_data_writer() {
  return cc.get_data_writer();
}



//
////sfxc includes
//#include "Integration_slice.h"
//
//Log_writer *TSC_log_writer;
//Data_writer *TSC_data_writer;
//INT64 TSC_startTS;
//
//// Initialise the Time Slice correlator.
//TScorrelator::TScorrelator(GenP& GenPrms, StaP* StaPrms,
//  Log_writer& lg_wrtr, 
//  Data_writer& dt_wrtr,
//  std::vector<Data_reader *> input_readers,
//  INT64 startTS)
//  //member initialisations
//  :dc(GenPrms, StaPrms, lg_wrtr), cc(GenPrms, dt_wrtr)
//{
//  TSC_log_writer = &lg_wrtr;
//  TSC_data_writer = &dt_wrtr;
//  TSC_startTS = startTS;
//}
//
//
//// Correlates all the segments (Nsegm2Avg) in the time slice.
//void TScorrelator::CorrelateTimeSlice(
//  StaP* StaPrms,
//  std::vector<Data_reader *> input_readers)
//{
//  char data[256];
//  sprintf(data, "%ld\n", TSC_startTS);
//  TSC_startTS++;
//  TSC_data_writer->put_bytes(strlen(data), data);
//  usleep(10000);
//}
//
