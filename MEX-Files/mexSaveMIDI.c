/**********************************************************************
 *                                                                    *
 * mexSaveMIDI.cpp                                                    *
 *                                                                    *
 * calls a computational function that saves a vector of sample       *
 * times as a MIDI file                                               *
 *                                                                    *
 * This is a MEX-file for MATLAB.                                     *
 *                                                                    *
 **********************************************************************/

#include "mex.h"
#include "SaveMIDI.h"

/*******************************************************************/
/*  mex function                                                   */
/*******************************************************************/
void mexFunction(int nlhs,  mxArray *plhs[],
		 int nrhs,  const mxArray *prhs[])
{ 
  double *times;
  int timesLength;
  
  double *louds;

  char *filename;
  int   filenameLength, status;

  /* Check for proper number of arguments. */
  if(nrhs != 3) 
  {
    mexPrintf("Usage: mexSaveMIDI (<times>, <louds-volume>, <output filename>");
    mexErrMsgTxt("three input arguments required: check ");
    return;
  }

  /*********************************/
  /* unpack vector input arguments */
  /*********************************/
  times = mxGetPr(prhs[0]);
  timesLength = mxGetNumberOfElements(prhs[0]);
  
  louds = mxGetPr(prhs[1]);


  /***********************************************************/
  /* all this mess just to get the frikken filename through  */
  /***********************************************************/

  /* input must be a string */
  if ( mxIsChar(prhs[2]) != 1)
    mexErrMsgTxt("Input must be a string.");
  
  /* input must be a row vector */
  if (mxGetM(prhs[2])!=1)
    mexErrMsgTxt("Input must be a row vector.");
  
  /* get the length of the input string */
  filenameLength = (mxGetM(prhs[2]) * mxGetN(prhs[2])) + 1;
  
  /* allocate memory for input and output strings */
  filename = mxCalloc(filenameLength, sizeof(char));
  status = mxGetString(prhs[2], filename, filenameLength);
  
  if(status != 0) 
    mexWarnMsgTxt("Not enough space. String is truncated.");
  
  /*******************************************************************/
  /* Call the subroutine to do the work                              */
  /*******************************************************************/
  saveMIDI(times, timesLength, louds, filename);
  
}


