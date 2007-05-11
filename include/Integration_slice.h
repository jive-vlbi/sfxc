/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef INTEGRATION_SLICE_H
#define INTEGRATION_SLICE_H

#include <types.h>
#include "Timer.h"
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
    Integration_slice(Log_writer &lg_wrtr);

    Integration_slice(GenP &GenPrms, StaP* StaPrms, Log_writer &lg_wrtr, 
       int ref_station);

    void set_parameters(GenP &GenPrms, StaP* StaPrms, int ref_station);
      

    /** assigns delay table for station number sn**/
    bool set_delay_table(int sn, DelayTable &delay_table);

    /** **/
    void set_sample_reader(int sn, Bits_to_float_converter *sample_reader);

    /** **/
    void set_data_writer(Data_writer *data_writer);

    /** **/
    bool init_reader(int sn, INT64 startIS);

    /**
     *  Set the start time and the duration of the correlation.
     *  NGHK: DISCUSS: Is this function needed? 
     *  DelayCorrection can find the start time in the data?
     **/
    void set_start_time(INT64 us_start);

    /** Correlates all the segments (Nsegm2Avg) in the time slice.
    \pre startIS: start time of current integration slice in usec wrt 00:00
    **/
    bool correlate();
    
    Data_writer &get_data_writer();

    Log_writer &get_log_writer();

  private:
    INT32           Nsegm2Avg; //nr of segments to average
    DelayCorrection dc;//class
    CorrelationCore cc;//class
    bool            parameters_set; // For debugging

    Log_writer      &log_writer;

};
#endif // INTEGRATION_SLICE_H
