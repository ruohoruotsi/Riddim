/**********************************************************************
 *                                                                    *
 * mexBandOnsets.cpp                                                  *
 *                                                                    *
 * calls a computational function that calculates the onsets          *
 * that occur in a specific band                                      *
 *                                                                    *
 * This is a MEX-file for MATLAB.                                     *
 *                                                                    *
 **********************************************************************/

#include <iostream>
#include <vector>

/**********************************************************************/
/* necessary when using C++ external MATLAB MEX functions             */
/**********************************************************************/

extern "C" {
#include "mex.h"
}

/* this must come IMMEDIATELY after the standard #include statements */
using namespace std;

#include "BandOnsets.h"

/*******************************************************************/
/*  mex function                                                   */
/*******************************************************************/
void mexFunction(int nlhs,  mxArray *plhs[],
		 int nrhs,  const mxArray *prhs[])
{ 
  int i, peakCount;
  double *B, *A, *Bdec, *Adec;     // filter coefficient vectors
  double *signal;                  // big arse input vector
  
  double srate, Io, timefrac, loudtimefrac, Fdec, thresholdValue; 
  int diffPrecision;
  int signalLength;
  int filterLength;
  
  /* output vectors    */
  double *peakTimesB;
  double *peakPressesB;
  double *peakBgPressesB;
  double *bankPressesB; 
  
  /* temporary storage */
  DoubleVector temp_peakTimesB;
  DoubleVector temp_peakPressesB;
  DoubleVector temp_peakBgPressesB;
  DoubleVector temp_bankPressesB;
  
  /* Check for proper number of arguments. */
  if(nrhs != 10) 
  {
    mexPrintf("Usage: bandOnsets( <Bbank>, <Abank>, <signal>,<srate>");
    mexPrintf("<Io>, <timefrac>, <loudtimefrac>, <diffPrecision>, ");
    mexPrintf("<Fdec>, <Bdec>, <Adec> )");
    
    mexErrMsgTxt("Eleven inputs arguments required: check ");
    return;
  }
  
  /* unpack vector input arguments */
  signal = mxGetPr(prhs[0]);
  Bdec = mxGetPr(prhs[7]);
  Adec = mxGetPr(prhs[8]);
  
  /* unpack scalars input arguments */
  srate = mxGetScalar(prhs[1]);
  Io = mxGetScalar(prhs[2]);
  timefrac = mxGetScalar(prhs[3]);
  loudtimefrac = mxGetScalar(prhs[4]);
  diffPrecision = mxGetScalar(prhs[5]);
  Fdec = mxGetScalar(prhs[6]);
  thresholdValue = mxGetScalar(prhs[9]);  

  /*  misc arguments necessary for calculation */
  signalLength = mxGetNumberOfElements(prhs[0]);
  
  /*******************************************************************/
  /* Call the subroutine to do the work                              */
  /*******************************************************************/
  BandOnsets *b = new BandOnsets();
  b->getBandOnsets(temp_peakTimesB,     // output arguments
                   temp_peakPressesB,              
                   temp_peakBgPressesB, 
                   temp_bankPressesB, 
                   &peakCount,          // input arguments
                   signal, 
                   srate, timefrac,     
                   loudtimefrac, diffPrecision, 
                   Fdec, Bdec, Adec, 
                   signalLength,
                   thresholdValue);       // misc arguments 
  
  /*  allocate memory for the output vectors according to MEX  */
  plhs[0] = mxCreateDoubleMatrix(1, temp_peakTimesB.size(), mxREAL); 
  plhs[1] = mxCreateDoubleMatrix(1, temp_peakPressesB.size(), mxREAL); 
  plhs[2] = mxCreateDoubleMatrix(1, temp_peakBgPressesB.size(), mxREAL); 
  plhs[3] = mxCreateDoubleMatrix(1, temp_bankPressesB.size(), mxREAL); 
  
  peakTimesB = mxGetPr(plhs[0]);
  peakPressesB = mxGetPr(plhs[1]);
  peakBgPressesB = mxGetPr(plhs[2]);
  bankPressesB = mxGetPr(plhs[3]);  

  /*******************************************************************/
  /* dump variable length vectors to MEX output                      */
  /*******************************************************************/

  /* this should be an assert */
  // use an mxAssert here 
  
  if(temp_peakTimesB.size() != temp_peakPressesB.size() &&
     temp_peakTimesB.size() != temp_peakBgPressesB.size())
  {
    mexPrintf("ACHTUNG!!!! peaktimes != presses != bgpresses");
  }

  /* create iterators for each vector */
  DoubleVector::iterator itimes = temp_peakTimesB.begin();
  DoubleVector::iterator ipresses = temp_peakPressesB.begin();
  DoubleVector::iterator ibgpresses;
  DoubleVector::iterator ibankpresses;

  /* these vectors should all be the same size  */
  /* so take one and use that to count thru all */
  for(i = 0, ibgpresses = temp_peakBgPressesB.begin();
      ibgpresses != temp_peakBgPressesB.end();
      i++, itimes++, ipresses++, ibgpresses++)
  {
    peakTimesB[i] = *itimes;
    peakPressesB[i] = *ipresses;
    peakBgPressesB[i] = *ibgpresses;
  }
  
  /* this guy's size is potentially different */
  for(i = 0,  ibankpresses = temp_bankPressesB.begin();
      ibankpresses != temp_bankPressesB.end();
      i++,ibankpresses++)
  {
    bankPressesB[i] = *ibankpresses;
  }
  
  /*  no memory leak here  */
  delete b;
}

