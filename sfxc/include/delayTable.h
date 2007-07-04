/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class definitions for delay table
 */
#ifndef DELAYTABLE_H
#define DELAYTABLE_H

#include <types.h>
#include <vector>
#include <genPrms.h>

class MPI_Transfer;

class DelayTable
{
  friend class MPI_Transfer;
  
  public:
    //constructor, set default values 
    DelayTable();

    //destructor
    ~DelayTable();
    
    bool operator==(const DelayTable &other) const;

    void set_cmr(GenP GenPrms);

    //read the delay table, do some checks and
    //calculate coefficients for parabolic interpolation
    int readDelayTable(char *delayTableName);

    //calculate the delay for the delayType at time in microseconds
    double calcDelay(int64_t time, int delayType) const;

    enum delayType {Cdel, Mdel, Rdel, Fdel};



  private:
    void reserve_data();

    //get the next line from the delay table file
    int getDelayLine(FILE *fp, int64_t &t, 
      double &c, double &m, double &r, double &f);

    //calculate the parabolic coefficients A, B and C
    int parabCoefs (int64_t t0,int64_t t1,int64_t t2,
      double d1,double d2,double d3,
      double& A,double& B,double& C);

    int64_t  ndel; // number of parabolic delay functions covering start until stop
    
    int64_t  startDT;       // start time delaytable in micro seconds
    int64_t  stepDT;        // time step in delaytable in micro seconds
    double one_over_stepDT;
    std::vector<double> cA, cB, cC; // Y = aX^2 + bX + c
    std::vector<double> mA, mB, mC; // per read delay line one series of these coefficients
    std::vector<double> rA, rB, rC;
    std::vector<double> fA, fB, fC;
    
    int cde, mde, rde; //switches which determine which columns of delay table are used
};


#endif // DELAYTABLE_H
