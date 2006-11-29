/**
CVS keywords
$Author $
$Date$
$Name$
$Revision$
$Source$

Class definitions for delay table

Author     : RHJ Oerlemans
StartDate  : 20061115
Last change: 20061115

**/

#include <types.h>

class DelayTable
{

  private:

    //
    INT64  ndel; // number of parabolic delay functions covering start until stop
    
    INT64  startDT;       // start time delaytable
    INT64  stepDT;        // time step in delaytable
    double *cA, *cB, *cC; // Y = aX^2 + bX + c
    double *mA, *mB, *mC; // per read delay line one series of these coefficients
    double *rA, *rB, *rC;
    double *fA, *fB, *fC;
    
        
  public:

    //default constructor, set default values 
    DelayTable();

    //destructor
    ~DelayTable();
    
    //read the delay table, do some checks and
    //calculate coefficients for parabolic interpolation
    int readDelayTable(char *delayTableName, INT64 start, INT64 stop, INT64 BufTime);

    //calculate the delay for the delayType at time in microseconds
    double calcDelay(double time, int delayType);

    enum delayType {Cdel, Mdel, Rdel, Fdel};

};


