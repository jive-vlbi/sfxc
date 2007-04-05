/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * timer structure and prototypes
 */


typedef struct {
    clock_t CPUbegin;
    clock_t CPUend;
    time_t Tbegin;
    time_t Tend;
    char   ID[100];
} timer, *ptimer;


//Start a timer
void tmrBegin(ptimer tmr, Log_writer &log_writer);

//Stop a timer
void tmrEnd(ptimer tmr, Log_writer &log_writer);

