/*
sfxc_SC = sfxc Single Core
sfxc    = software FX correlator
FX      = first FFT then correlate

usage: sfxc_SC correlator_control_file 

Source       sfxc_SC.cc
Author       RHJ Oerlemans
Started      20070209
Last change  20070209
*/

#include "sfxc_SC.h"

#define SEED 10

//TODO RHJO make local after InData and ProcessData are depricated
//global variables
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;
//station parameters class, declaration and default settings
StaP StaPrms[NstationsMax];
// used for randomising numbers for Headers in Mk4 file
UINT32 seed;

int main(int argc, char *argv[])
{
  //initialise log writer for run time messagae
  Log_writer_cout log_writer(0,false);
  set_log_writer(log_writer);

  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " correlator_control_file" << std::endl;
    exit(1);
  }

  //make fixed seeding or time based seeding a control file option
  seed = SEED;
  log_writer << std::endl << "WARNING seed=" << seed << endl;

  //set the control file name
  char   ctrlFile[lineLength]; // control file name
  strcpy(ctrlFile,argv[1]);

  //declaration and default settings run parameters
//  RunP RunPrms;
  
  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile, log_writer) != 0) {
    log_writer << "ERROR: Control file "
               << ctrlFile <<", program aborted.\n";
    return -1;
  }

  //set the message level and run mode
  log_writer.set_messagelevel(RunPrms.get_messagelvl());
  log_writer.set_interactive(RunPrms.get_interactive());

  //show version information and control file info
  log_writer << "Source " << __FILE__ 
             << " compiled at: " << __DATE__ << " "<<__TIME__<<"\n\n";
  
  
  //check run control parameters, optionally show them
  if (RunPrms.check_params(log_writer) != 0) {
    log_writer.
    message(0,"ERROR: Run control parameter, program aborted.\n");
    return -1;
  }
  
  log_writer.ask_continue();

  //declaration and default settings general parameters
//  GenP GenPrms;
    
  //parse control file for general parameters
  if (GenPrms.parse_ctrlFile(ctrlFile, log_writer) != 0) {
    log_writer << "ERROR: Control file "
               << ctrlFile <<", program aborted.\n";
    return -1;
  }


  //check general control parameters, optionally show them
  if (GenPrms.check_params(log_writer) != 0) {
    log_writer.
    message(0,"ERROR: General control parameter, program aborted.\n");
    return -1;
  }
  
  log_writer.ask_continue();

  //set the output writer
  Data_writer_file data_writer(GenPrms.get_corfile());
  set_data_writer(data_writer); 

  //get the number of stations
  int    nstations;
  nstations = GenPrms.get_nstations();
    
  //station parameters class, declaration and default settings
//  StaP StaPrms[NstationsMax];
  
  //parse the control file for all station parameters
  for (int i=0; i<nstations; i++)
    if (StaPrms[i].parse_ctrlFile(ctrlFile, i, log_writer) != 0 ) {
      log_writer << "ERROR: Control file "
                 << ctrlFile <<", program aborted.\n";
      return -1;
    }
    
  //check station control parameters, optionally show them
  for (int i=0; i<nstations; i++){
    if (StaPrms[i].check_params(log_writer) != 0 ) {
      log_writer.
      message(0,"ERROR: Station control parameter, program aborted.\n");
      return -1;
    }
    log_writer.ask_continue();
  }

  //initialise input readers
  // NGHK: Has to be a pointer or a reference,
  //       since Data_reader is an abstract class
  std::vector<Data_reader *> input_readers;
  for (int i=0; i<nstations; i++) {
    input_readers.push_back(new Data_reader_file(StaPrms[i].get_mk4file()));
  }


  //display and check mk4file header info for start time set in ccf
  if (show_MK4_headers(input_readers) !=0) {
    log_writer.message(0,"ERROR: show_MK4_headers, program aborted.\n");
    return -1;
  }

  
  //process the mk4file data
  if ( RunPrms.get_runoption() == 1 ) {

    int nTS=GenPrms.get_usDur()/GenPrms.get_usTime2Avg(), TS=0;
    
    //initialise start of first time slice,
    //which is start of scan wrt to 00:00 in usec
    INT64 startTS=GenPrms.get_usStart();

    //initialise time slice correlator
    TScorrelator tsc(GenPrms, StaPrms, log_writer, data_writer, input_readers, startTS);

    //while still time slices and data are available
    while (startTS < GenPrms.get_usStart() + GenPrms.get_usDur()
           // && data_available TODO RHJO implement
           )
    {
      //TODO RHJO test and debug code
      cout << "\nTS/nTS " << setw(2) << ++TS << "/" << nTS <<
       " startTS " << startTS << endl;
      //process the next time slice:
      tsc.CorrelateTimeSlice(StaPrms, input_readers);
      //set start of next time slice to: start of time slice + time to average
      startTS += GenPrms.get_usTime2Avg(); //in usec
    }
  
  }

}
