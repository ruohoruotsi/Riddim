/**********************************************************************
 *                                                                    *
 * mexFillHistorgrams.cpp                                             *
 *                                                                    *
 * This is a MEX-file for MATLAB.                                     *
 *                                                                    *
 **********************************************************************/

#include <iostream>
#include <vector>
#include <math.h>

/**********************************************************************/
/* necessary when using C++ external MATLAB MEX functions             */
/**********************************************************************/

extern "C" {
#include "mex.h"
#include "dspUtils.h"
}

/* this must come IMMEDIATELY after the standard #include statements */
using namespace std;

#include "BandOnsets.h"


/*******************************************************************/
/*       returns the per histo GCD error function                  */
/*******************************************************************/
double* getRemainderError(double *ioiHisto, 
                          double gcdRemainderErrLength,
                          double ioi_rez,
                          double histLength)
{
  int i, j, k;
  
  // local variables
  double q, numerator, mr;

  double *remainderError = new double[(int)gcdRemainderErrLength];
  double denominator = 0.0;
  
  // sum up ioiHisto
  for(i = 0; i < histLength; i++)
  {
    denominator += ioiHisto[i];
  }
  
  // IO Kludge take things to millisecs for the timebeing
  ioi_rez = ioi_rez/48000;     
  
  /***************************************************************/
  /* calculate the remainderError function for current ioiHisto  */
  /***************************************************************/
  for(j = 0; j < gcdRemainderErrLength; j++)
  {
    q = (j+1) * ioi_rez;
    numerator = 0.0;
    
    // k*ioi_rez gives the kth IOI position in the histogram 
    for(k = 0; k < histLength; k++)
    {
      mr = fmod(((k+1) * ioi_rez/q) + 0.5, 1) - 0.5; // mr = modified residual
      numerator += ioiHisto[k] * mr * mr;
    }
    
    if( denominator > 0)
    {
      remainderError[j] = numerator/denominator;
    } 
  } 
  
  return remainderError; // the caller of this method shouldn't forget
                         // to deallocate this mofo - otherwise a nice
                         // memory leak! trankwil ...
}


/*******************************************************************/
/*       mex function                                              */
/*******************************************************************/
void mexFunction(int nlhs,  mxArray *plhs[],
		 int nrhs,  const mxArray *prhs[])
{ 
  double *onsetVector;
  int onsetVectorSize;
  int sr;

  /* Check for proper number of arguments. */
  if(nrhs != 2) 
  {
    mexPrintf("Usage: mexFillHistograms (<onsetVector>, <samplerate>)");
    mexErrMsgTxt("one input arguments required: check ");
    return;
  }
  
  /* unpack vector input arguments */
  onsetVector = mxGetPr(prhs[0]);
  onsetVectorSize = mxGetNumberOfElements(prhs[0]);
  sr = mxGetScalar(prhs[1]);

  
  /********************************************************************/
  /********************************************************************/
  /*                 set up analysis conditions                       */
  /********************************************************************/
  /********************************************************************/

  double ioi_rez = (5.0/1000.0) * sr; //  5 ms max_rez = 40 ms min_rez = 1 ms
  double minTatum = sr/100.0;        //  10 ms 
  double maxTatum = sr;              //  1 second

  
  // in ms becos it is used in leaky integration calculation
  double histHalfLife = 1300.0; 
  double frameLengthInMS = 500.0; // easy because we think in seconds not samples!
  
  // since this is non-realtime and the histogram is updated every
  // timepassed milliseconds, the timepassed/histHalfLife ratio is key
  // to how much info is maintained from one histogram update to another
  double timePassed = 500.0;
  
  // ioi_rez is the "histogram" sample rate, 1/ioi_rez = fs
  // gives the width of each histogram bin
  double histLength            = ceil(maxTatum/ioi_rez);
  double gcdRemainderErrLength = ceil(maxTatum/ioi_rez);
  
  double frameLengthInSamples = (frameLengthInMS/1000.0) * sr;
  
  //  declare static arrays to hold histogram info 
  double* fillHisto = new double[histLength];
  double* ioiHisto = new double[histLength];
  double* remainderError; // memory allocated lower down
  
  // fifo
  DoubleVector fifo(500);
  int fifoSize = 0;
  int j = 0;  
  
  double* histogramVector;  // our "matrix"

  plhs[0] = mxCreateDoubleMatrix(onsetVectorSize, histLength, mxREAL); 
  histogramVector = mxGetPr(plhs[0]);

  int count = 0;
  for (int i = 0; i < onsetVectorSize; i++)
  {
    
    // go thru each onset and put in in a fifo
    // each update of the fifo spawns a recalculation of
    // a fill histogram that is added w/ leaky to a vector
    // of histograms tracking the progress of the llp.

    double now = onsetVector[i];
    for(int j = 0; j < fifoSize; j++)
    {
      double ioi = now - onsetVector[j]; 
      double ioi_index = floor(ioi/ioi_rez);  // quantization happens here

      // increment fill_histogram	  
      if (ioi_index < histLength)
      {
        fillHisto[(int)ioi_index]++;
      }      
    }

    // add code here to chop off earliest onsets
    // if fifoSize == maxfifoSize;

    // store onsets
    fifo.push_back(now);
    fifoSize++;

    // add fillHisto to ioiHisto implement leaky integrator
    double leakCoeff = pow(.5, (timePassed  /  histHalfLife));
    double fillCoeff = (1 - leakCoeff) / leakCoeff;
    
    for(int m = 0; m < histLength; m++)
    {
      ioiHisto[m] = 
        leakCoeff * ioiHisto[m] + 
        fillCoeff * fillHisto[m];
      
      // clear out fillHisto as we go along
      fillHisto[m] = 0;
    }
    
    // convert histograms into the GCD Error Function
    remainderError = getRemainderError(ioiHisto, 
                                       gcdRemainderErrLength,
                                       ioi_rez,
                                       histLength);

    //    createMAT("debug_histo.mat", remainderError, histLength);    

    // accumulate ErrorFunctions in histogramVector for quantum analysis
    for (int c = 0;  c < histLength;  c++ )
    {
      // matlab what the fuck?!!!
      histogramVector[onsetVectorSize*c + count] = remainderError[c];
    }
    count++;

    // recycle since it'll be re-allocated on the next iteration
    delete [] remainderError;

 } 
  
  // export ioiHisto to mat file so we can check it.

  
  // clean up shop ...
  delete [] fillHisto;
  delete [] ioiHisto;
}

