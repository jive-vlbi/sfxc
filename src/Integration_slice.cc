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
  StaP *StaPrms,
  Log_writer &lg_wrtr)
  //member initialisations
  :dc(GenPrms, StaPrms, lg_wrtr), cc(GenPrms)
{
  Nsegm2Avg = 2 * GenPrms.get_bwfl() / GenPrms.get_n2fft();
  Nsegm2Avg = (INT32) (GenPrms.get_time2avg() * Nsegm2Avg);
}



//pass the delay table
void Integration_slice::set_delay_table(int i, DelayTable &delay_table)
{
  dc.set_delay_table(i,delay_table);
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
void Integration_slice::init_reader(int sn, INT64 startIS)
{
  dc.init_reader(sn,startIS);
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
