/*
Author     : RHJ Oerlemans
StartDate  : 20070201
Last change: 20070209
*/


//sfxc includes
#include "TScorrelator.h"

// Initialise the Time Slice correlator.
TScorrelator::TScorrelator(GenP& GenPrms, StaP* StaPrms,
  Log_writer& lg_wrtr, 
  Data_writer& dt_wrtr,
  std::vector<Data_reader *> input_readers,
  INT64 startTS)
  //member initialisations
  :dc(GenPrms, StaPrms, lg_wrtr/*,input_readers*/), cc(GenPrms, dt_wrtr)
{
  Nsegm2Avg = 2 * GenPrms.get_bwfl() / GenPrms.get_n2fft();
  Nsegm2Avg = (INT32) (GenPrms.get_time2avg() * Nsegm2Avg);
  dc.init_readers(StaPrms, input_readers, startTS);
}


// Correlates all the segments (Nsegm2Avg) in the time slice.
void TScorrelator::CorrelateTimeSlice(
  std::vector<Data_reader *> input_readers)
{  
  //TODO RHJO test/debug code
  float TenPct=Nsegm2Avg/10.0, i=0;
  cout << "Nsegm2Avg " << Nsegm2Avg << endl;
  
  //zero accumulation accxps array and norms array.
  cc.init_time_slice();

  //process all the segments in the Time Slice (=Time to Average)
  for (INT32 segm = 0 ; segm < Nsegm2Avg ; segm++){

    //fill the current segment in cc with delay corrected data from dc
    dc.fill_segment(input_readers);
    //do the correlation for current segment.
    cc.correlate_segment(dc.get_segment());

    //TODO RHJO test/debug code
    if ( floor((segm+1)/TenPct) == i+1 ){
      i++;
      cout << "segm=" << setw(8) << segm << " " <<
      setw(3) << i*10 << " % of current TS processed\n";
    }
      
  }
  
  //normalise the accumulated correlation results
  cc.average_time_slice();

  //write the correlation result for the current time slice
  cc.write_time_slice();


}

