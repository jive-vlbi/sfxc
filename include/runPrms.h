/**
Class definitions for Run Parameters

Author     : RHJ Oerlemans
StartDate  : 12-09-2006
Last change: 10-11-2006

**/


class RunP
{

  private:

    //parameters which control how the program is run
    int  messagelvl;      //message level
    int  interactive;     //run interactive or automatically
    int  runoption;       //run option, default 1=complete
    int  ncores;          //number of computational cores, default 1
    
        
  public:

    //default constructor, set default values 
    RunP();

    //parse control file for run parameters
    int parse_ctrlFile(char *ctrlFile);

    //check run parameters
    int check_params();

    //get functions
    int get_messagelvl();
    int get_interactive();
    int get_runoption();
    int get_ncores();   

};


