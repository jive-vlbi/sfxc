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
void tmrBegin(ptimer tmr);

//Stop a timer
void tmrEnd(ptimer tmr);

