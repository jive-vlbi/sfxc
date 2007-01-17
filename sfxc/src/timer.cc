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

// NGHK: remove this, for now to have access to the Log_writer  
#include <ProcessData.h>

#include "timer.h"

//Start a timer
void tmrBegin(ptimer tmr)
{
    tmr->CPUbegin=clock();
    tmr->Tbegin=time(NULL);
    log_writer << "**** Timer for " << tmr->ID << "started\n";
}


//Stop a timer
void tmrEnd(ptimer tmr)
{
    double T_elapsed, CPU_elapsed;
    
    tmr->Tend=time(NULL);
    tmr->CPUend=clock();
    
    CPU_elapsed=((double) (tmr->CPUend - tmr->CPUbegin))/CLOCKS_PER_SEC;
    T_elapsed=difftime(tmr->Tend, tmr->Tbegin);
    log_writer << "**** Timer for " << tmr->ID << "stopped\n";
    log_writer << "CPU elapsed " << CPU_elapsed << " seconds Total " << T_elapsed << " seconds\n";
}


