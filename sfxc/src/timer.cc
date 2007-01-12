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

#include "timer.h"

//Start a timer
void tmrBegin(ptimer tmr)
{
    tmr->CPUbegin=clock();
    tmr->Tbegin=time(NULL);
    printf("\n**** Timer for %s started\n",tmr->ID);
}


//Stop a timer
void tmrEnd(ptimer tmr)
{
    double T_elapsed, CPU_elapsed;
    
    tmr->Tend=time(NULL);
    tmr->CPUend=clock();
    
    CPU_elapsed=((double) (tmr->CPUend - tmr->CPUbegin))/CLOCKS_PER_SEC;
    T_elapsed=difftime(tmr->Tend, tmr->Tbegin);
    printf("\n**** Timer for %s stopped, ",tmr->ID);
    printf("CPU elapsed %g seconds Total %g seconds\n",
    CPU_elapsed, T_elapsed);
}


