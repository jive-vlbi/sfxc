/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

General purpose functions

Author     : RHJ Oerlemans
StartDate  : 20060841
Last change: 20061114

*/

#include <types.h>

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//the class definitions and function definitions
#include "runPrms.h"
//global variables
extern RunP  RunPrms;

//*****************************************************************************
//continue or stop
//*****************************************************************************
void askContinue(void)
{
  char repl[2]; // user reply character
  
  cout << "\nEnter c to continue, any other character to stop: ";
  scanf("%s",repl);
  if (strcmp(repl,"c")!=0) {
    cout << "Application stopped by user!\n";
    exit(0);
  }
  cout << endl;
}




//*****************************************************************************
//
//*****************************************************************************
int getLongVal(char *key, char *val, char *skey, int& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtol(val, &endp, 10);
    if(RunPrms.get_messagelvl() > 1 ) {
      cout << "getLongVal: " << skey <<" "<< sval << endl;
    }
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into long\n",key);
      return -1;
    }
  }
  return 0;
}



//*****************************************************************************
//
//*****************************************************************************
int getINT64Val(char *key, char *val, char *skey, INT64& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtoll(val, &endp, 10);
    if(RunPrms.get_messagelvl() > 1 ) {
      cout << "getINT64Val: " << skey <<" "<< sval << endl;
    }
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into long\n",key);
      return -1;
    }
  }
  return 0;
}


//*****************************************************************************
//
//*****************************************************************************
int getFloatVal(char *key, char *val, char *skey, float& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtod(val, &endp);
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into float\n",key);
      return -1;
    }
  }
  return 0;
}


//*****************************************************************************
//
//*****************************************************************************
int str2int(char *val, int& sval)
{
  char *endp;
  
  sval = strtol(val, &endp, 10);
  if (endp == val) {
    fprintf(stderr,
      "**** Unable to convert string %s into long\n",val);
    return -1;
  }
  return 0;  
}


//*****************************************************************************
//  irbit2: random seeding
//  See Numerical Recipes
//  primitive polynomial mod 2 of order n produces 2^n - 1 random bits
//*****************************************************************************
int irbit2(UINT32 *iseed){
  #define IB1 1
  //  #define IB2 2
  #define IB4 8
  //  #define IB5 16
  #define IB6 32 
  //  #define IB18 131072
  #define IB30 536870912
  #define MASK (IB1+IB4+IB6)
 
    
  if (*iseed & IB30) {
    *iseed=((*iseed ^ MASK) << 1) | IB1;
    return 1;
  } else {
    *iseed <<= 1;
    return 0;
  }
  #undef MASK
  #undef IB30
  //  #undef IB18
  #undef IB6
  //  #undef IB5
  #undef IB4
  //  #undef IB2
  #undef IB1
}




