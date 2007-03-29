#ifndef INTEGRATION_SLICE_H
#define INTEGRATION_SLICE_H

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
class Integration_slice
{
  public:
     
    /** Initialise the Time Slice correlator.  **/
    Integration_slice(
      GenP &GenPrms,  
      StaP* StaPrms,
      Log_writer &lg_wrtr);

    /** assigns delay table for station number sn**/
    void set_delay_table(int sn, DelayTable &delay_table);

    /** **/
    void set_data_reader(int sn, Data_reader *data_reader);

    /** **/
    void set_data_writer(Data_writer *data_writer);

    /** **/
    void init_reader(int sn, StaP &StaPrms, INT64 startIS);

    /** Correlates all the segments (Nsegm2Avg) in the time slice.
    \pre startIS: start time of current integration slice in usec wrt 00:00
    **/
    void correlate();
    

  private:
    INT32           Nsegm2Avg; //nr of segments to average
    DelayCorrection dc;//class
    CorrelationCore cc;//class

};
#endif // INTEGRATION_SLICE_H
