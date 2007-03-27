/*
header file to be included in sfxc_SC.cc

Source       sfxc_SC.h
Author       RHJ Oerlemans
Started      20070213
Last change  20070213
*/

#ifndef SFXC_SC_H
#define SFXC_SC_H

#include <types.h>

//c includes
#include <string.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;

//constants to be included in various source files
#include "constPrms.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "InData.h"
#include "Log_writer.h"
#include "Data_writer.h"
#include "Correlate_integration_slice.h"

#include <Data_reader_file.h>
#include <Data_writer_file.h>
#include <Log_writer_cout.h>

#endif // SFXC_SC_H

void set_log_writer(Log_writer &writer);
Log_writer &get_log_writer();
  
void set_data_writer(Data_writer &writer);
Data_writer &get_data_writer();
