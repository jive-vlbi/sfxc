/*
CVS keywords
$Id$
$Name$

timer structure and prototypes
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

