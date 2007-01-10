#ifndef DELAYTABLE_H
#define DELAYTABLE_H
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
#include <vector>

class MPI_Transfer;

class DelayTable
{
  friend class MPI_Transfer;
  
  public:
    //default constructor, set default values 
    DelayTable();

    //destructor
    ~DelayTable();
    
    bool operator==(const DelayTable &other) const;
    
    //read the delay table, do some checks and
    //calculate coefficients for parabolic interpolation
    int readDelayTable(char *delayTableName, INT64 start, INT64 stop, INT64 BufTime);
    //read the delay table, do some checks and
    //calculate coefficients for parabolic interpolation
    int readDelayTable(char *delayTableName, INT64 BufTime);

    //calculate the delay for the delayType at time in microseconds
    double calcDelay(double time, int delayType) const;

    enum delayType {Cdel, Mdel, Rdel, Fdel};



  private:
    void reserve_data();
    INT64  ndel; // number of parabolic delay functions covering start until stop
    
    INT64  startDT;       // start time delaytable
    INT64  stepDT;        // time step in delaytable
    std::vector<double> cA, cB, cC; // Y = aX^2 + bX + c
    std::vector<double> mA, mB, mC; // per read delay line one series of these coefficients
    std::vector<double> rA, rB, rC;
    std::vector<double> fA, fB, fC;
};


#endif // DELAYTABLE_H
