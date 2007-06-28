/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class definitions for Run Parameters
 */

#ifndef RUN_PRMS_H
#define RUN_PRMS_H

#include "Log_writer.h"

class MPI_Transfer;

class RunP
{
  friend class MPI_Transfer;

  public:

    //default constructor, set default values 
    RunP();

    //parse control file for run parameters
    int parse_ctrlFile(const char *ctrlFile, Log_writer &log_writer);

    //check run parameters
    int check_params(Log_writer &log_writer) const;

    //get functions
    
    /** The message level.
     * -# only error and abort messages will appear
     * -# 0 + higher level progress and warning messages
     * -# 1 + lower-level warning and progress messages + information messages
    **/
    int get_messagelvl() const;
    /** Ask for confirmation to continue after completion of a processing step.
     * Enabling the interactive mode is only useful when message level is 1 or higher.
     * DO NOT RUN INTERACTIVE WHEN USING MORE THAN 1 CORE/PROCESS
     **/
    int get_interactive() const;
    /** Run options.
     * - 0 : determine the file offset to get to the START. Usually this option
     *        is run in combination with MESSAGELVL>1 and INTERACTIVE=1
     * - 1 : execute all main processing steps: offset, unpack, delay, correlate
     **/
    int get_runoption() const;
    /** Reference station
     * -1 and >= nstations : correlate all auto and cross base lines
     * 0 <= ref_station < nstation : correlate all autos and only crosses against reference
     *
     * @pre: station_nr is 0 or 1, since we want to have two reference
     *       stations if we correlate two polarisations
     **/
    int get_ref_station(int station_nr) const;

  private:
    //parameters which control how the program is run
    int  messagelvl;      //message level
    int  interactive;     //run interactive or automatically
    int  runoption;       //run option, default 1=complete
    int  ref_station1;    //-1 or > nstations all correlations. 
                          //0<=ref_station<nsations use reference station
    int  ref_station2;    //-1 or > nstations all correlations. 
                          //0<=ref_station<nsations use reference station
};


#endif // RUN_PRMS_H
