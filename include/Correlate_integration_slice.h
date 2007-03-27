#ifndef CORRELATE_INTEGRATION_SLICE_H
#define CORRELATE_INTEGRATION_SLICE_H

/*
Author     : RHJ Oerlemans
StartDate  : 20070201
Last change: 20070201

*/

#include <types.h>
#include "genPrms.h"
#include "staPrms.h"
#include "DelayCorrection.h"
#include "CorrelationCore.h"
#include <Data_reader_file.h>

/** TScorrlelator correlates one time slice of data for all stations. **/
class Correlate_integration_slice
{
  public:
     
    /** Initialise the Time Slice correlator.  **/
    Correlate_integration_slice(GenP& GenPrms,  StaP* StaPrms,
      Log_writer& lg_wrtr, 
      Data_writer& dt_wrtr,
      std::vector<Data_reader *> input_readers,//TODO RHJO reference
      INT64 startTS);

    /** Correlates all the segments (Nsegm2Avg) in the time slice.
    \pre startTS: start time of current time slice in usec wrt 00:00
    **/
    void CorrelateTimeSlice( std::vector<Data_reader *> input_readers); //TODO RHJO reference
    

  private:
    INT32           Nsegm2Avg; //nr of segments to average
    DelayCorrection dc;//class
    CorrelationCore cc;//class

};
#endif // CORRELATE_INTEGRATION_SLICE_H
