#ifndef TSCORRELATOR_H
#define TSCORRELATOR_H

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
class TScorrelator
{
  public:
     
    /** Initialise the Time Slice correlator.  **/
    TScorrelator(GenP& GenPrms,  StaP* StaPrms,
      Log_writer& lg_wrtr, 
      Data_writer& dt_wrtr,
      std::vector<Data_reader *> input_readers,
      INT64 startTS);

    /** Correlates all the segments (Nsegm2Avg) in the time slice.
    \pre startTS: start time of current time slice in usec wrt 00:00
    **/
    void CorrelateTimeSlice(
      StaP* StaPrms,
      std::vector<Data_reader *> input_readers);
    

  private:
    INT32           Nsegm2Avg; //nr of segments to average
    DelayCorrection dc;//class
    CorrelationCore cc;//class

};
#endif // TSCORRELATOR_H
