#ifndef RUN_PRMS_H
#define RUN_PRMS_H
/**
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class definitions for Run Parameters

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20061114

**/

#include "Log_writer.h"

class MPI_Transfer;

class RunP
{
  friend class MPI_Transfer;

  public:

    //default constructor, set default values 
    RunP();

    //parse control file for run parameters
    int parse_ctrlFile(char *ctrlFile, Log_writer &log_writer);

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

  private:
    //parameters which control how the program is run
    int  messagelvl;      //message level
    int  interactive;     //run interactive or automatically
    int  runoption;       //run option, default 1=complete
};


#endif // RUN_PRMS_H
