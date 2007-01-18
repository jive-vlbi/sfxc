/*
CVS keywords
$Id$
$Name$

this file contains timer functions
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "Log_writer.h"

#include "timer.h"

//Start a timer
void tmrBegin(ptimer tmr, Log_writer &log_writer)
{
    tmr->CPUbegin=clock();
    tmr->Tbegin=time(NULL);
    log_writer << "**** Timer for " << tmr->ID << "started\n";
}


//Stop a timer
void tmrEnd(ptimer tmr, Log_writer &log_writer)
{
    double T_elapsed, CPU_elapsed;
    
    tmr->Tend=time(NULL);
    tmr->CPUend=clock();
    
    CPU_elapsed=((double) (tmr->CPUend - tmr->CPUbegin))/CLOCKS_PER_SEC;
    T_elapsed=difftime(tmr->Tend, tmr->Tbegin);
    log_writer << "**** Timer for " << tmr->ID << "stopped\n";
    log_writer << "CPU elapsed " << CPU_elapsed << " seconds Total " << T_elapsed << " seconds\n";
}


