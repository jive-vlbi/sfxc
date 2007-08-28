/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * usage: sfxc_SC correlator_control_file 
 */

#include "sfxc_SC.h"
#include <Channel_extractor.h>
#include <Channel_extractor_mark4.h>
#include <Bits_to_float_converter.h>

#ifdef SFXC_PRINT_DEBUG
extern int RANK_OF_NODE;
#endif

int main(int argc, char *argv[])
{
#ifdef SFXC_PRINT_DEBUG
  // for DEBUG_MSG
  RANK_OF_NODE = 0;
#endif

  //initialise log writer for run time messages
  Log_writer_cout log_writer(0,false);
  set_log_writer(log_writer);


  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " correlator_control_file" << endl;
    exit(1);
  }

  //make fixed seeding or time based seeding a control file option
  set_seed(10);
  log_writer(1) << endl << "WARNING fixed seed=10\n";

  //set the control file name
  char   ctrlFile[lineLength]; // control file name
  strcpy(ctrlFile,argv[1]);

  //declaration and default settings run parameters
  RunP RunPrms;
  
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
  log_writer(1) << "Source " << __FILE__ 
                << " compiled at: " << __DATE__ << " "<<__TIME__<<"\n\n";
  
  
  //check run control parameters, optionally show them
  if (RunPrms.check_params(log_writer) != 0) {
    log_writer.
    message(0,"ERROR: Run control parameter, program aborted.\n");
    return -1;
  }
  
  log_writer.ask_continue();

  //declaration and default settings general parameters
  GenP GenPrms;
    
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
  boost::shared_ptr<Data_writer_file> data_writer(new Data_writer_file(GenPrms.get_corfile()));

  //get the number of stations
  int    nstations;
  nstations = GenPrms.get_nstations();
    
  //station parameters class, declaration and default settings
  StaP StaPrms[NstationsMax];
  
  //parse the control file for all station parameters
  for (int i=0; i<nstations; i++) {
    StaPrms[i].set_genPrms(GenPrms);
    if (StaPrms[i].parse_ctrlFile(ctrlFile, i, log_writer) != 0 ) {
      log_writer <<"ERROR: Control file "<< ctrlFile <<", program aborted.\n";
      return -1;
    }
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

  
  //initialise start of first integration slice,
  //which is start of scan wrt to 00:00 in usec
  int64_t startIS=GenPrms.get_usStart();

  bool result;

  //initialise integration slice
  Integration_slice IntSlc(GenPrms, StaPrms, log_writer, 
                           RunPrms.get_ref_station(0), 
                           RunPrms.get_ref_station(1));  

  for (int sn=0; sn< nstations; sn++) {

    //Delay table initialisations
    Delay_table_akima delay_table;
    delay_table.set_cmr(GenPrms);//set delay table column switches
    string msg = string("DelTbl: ")+StaPrms[sn].get_delaytable();
    get_log_writer().message(2,msg);
    int retval = delay_table.open(StaPrms[sn].get_delaytable());
    if (retval != 0) {
      get_log_writer().message(0,"ERROR: when reading delay table.\n");
      return -1;
    }
    result = IntSlc.set_delay_table(sn,delay_table);//pass the delay table 
    if (!result) {
      log_writer.error("Could not read delay table");
      return -1;
    }

    //Data reader initialisations
    boost::shared_ptr<Data_reader> 
      data_reader(new Data_reader_file(StaPrms[sn].get_mk4file()));
    boost::shared_ptr<Channel_extractor> 
      ch_extractor(new Channel_extractor_mark4(data_reader, StaPrms[sn], 
                                               GenPrms.get_rndhdr()));

    //initialise readers to proper position
    result = ch_extractor->goto_time(startIS);

    if (result != 0) {
      std::stringstream msg; msg << "result: " << result;
      log_writer.error(msg);
      log_writer.error("Could not go to the right time stamp");
      return -1;
    }

    boost::shared_ptr<Bits_to_float_converter>
      sample_reader(new Bits_to_float_converter());
    sample_reader->set_bits_per_sample(StaPrms[sn].get_bps());
    sample_reader->set_channel_extractor(ch_extractor);
    
    IntSlc.set_sample_reader(sn,sample_reader);//pass the data reader
    result = IntSlc.init_reader(sn,startIS);

    //display and check mk4file header info for start time set in ccf
    //NGHK: commented out, use channel extracter later on
    //show_MK4_header(data_reader, startIS, StaPrms[sn], GenPrms);
    

    if (!result && !RunPrms.get_interactive()) {
      log_writer.error(StaPrms[sn].get_mk4file());
      log_writer.error("Could not initialise the data reader");
      return -1;
    }

  }

  IntSlc.set_data_writer(data_writer);//pass the data writer 
  
  Timer tmr_process_data;
  tmr_process_data.set_ID("data processing");  
  tmr_process_data.start(log_writer);

  //process the mk4file data
  if ( RunPrms.get_runoption() == 1 ) {

    int nIS=GenPrms.get_usDur()/GenPrms.get_usTime2Avg(), IS=0;
    
    //while still integration slices and data are available
    while (startIS < GenPrms.get_usStart() + GenPrms.get_usDur())
    {
      log_writer(1) << "\nIS/nIS=" << ++IS << "/" << nIS 
                    << " startIS=" << startIS << " usec"<< endl;

      result = IntSlc.correlate();

      if (!result) {
        // Error in correlation, exiting.
        log_writer.error("Error in correlating time slice, exiting.");
        return 1;
      }

      //set start of next time slice to: 
      //  start of integration slice + time to average
      startIS += GenPrms.get_usTime2Avg(); //in usec
    }
  }

  tmr_process_data.stop(log_writer);

}
