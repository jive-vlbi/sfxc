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

class MPI_Transfer;

class RunP
{
  friend class MPI_Transfer;

  public:

    //default constructor, set default values 
    RunP();

    //parse control file for run parameters
    int parse_ctrlFile(char *ctrlFile);

    //check run parameters
    int check_params() const;

    //get functions
    int get_messagelvl();
    int get_interactive();
    int get_runoption();

  private:
    //parameters which control how the program is run
    int  messagelvl;      //message level
    int  interactive;     //run interactive or automatically
    int  runoption;       //run option, default 1=complete
};


#endif // RUN_PRMS_H
