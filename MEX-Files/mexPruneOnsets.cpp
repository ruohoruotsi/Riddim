/**********************************************************************
 *                                                                    *
 * mexPruneOnsets.cpp                                                 *
 *                                                                    *
 * calls a computational function that prunes the onsets based on     *
 * human psychoacoustic limitations                                   *
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
  int i; 
  double *times;
  double *louds;
  double loudthresh;
  double mininterv;
  
  DoubleVector temp_acceptedIndexes;
  double *acceptedIndexes;
  
  /* Check for proper number of arguments. */
  if(nrhs != 4) 
  {
    mexPrintf("Usage: pruneOutOnsets (<louds>, <times>, <loudthresh>");
    mexPrintf("<mininterv>) ");
    mexErrMsgTxt("four inputs arguments required: check ");
    return;
  }
  
  /* unpack vector input arguments */
  times = mxGetPr(prhs[0]);
  louds = mxGetPr(prhs[1]);
  
  /* unpack scalars input arguments */
  loudthresh = mxGetScalar(prhs[2]);
  mininterv = mxGetScalar(prhs[3]);
  
  /*
    mexPrintf("the size of times = %d\n", mxGetNumberOfElements(prhs[0]));
    mexPrintf("the size of louds = %d\n", mxGetNumberOfElements(prhs[1]));
    mexPrintf("the loudthresh = %f\n", loudthresh);
    mexPrintf("the mininterv = %f\n", mininterv);
  */
  
  /*******************************************************************/
  /* Call the subroutine to do the work                              */
  /*******************************************************************/
  BandOnsets *b = new BandOnsets();
  b->getPrunedOnsets(times,
                     louds,
                     (int)mxGetNumberOfElements(prhs[1]),
                     loudthresh,
                     mininterv,
                     temp_acceptedIndexes);
  
  /*  allocate memory for the output vectors according to MEX  */
  plhs[0] = mxCreateDoubleMatrix(1, temp_acceptedIndexes.size(), mxREAL); 
  acceptedIndexes = mxGetPr(plhs[0]);
  
  
  /*******************************************************************/
  /* dump accepted indexes to MEX output                             */
  /*******************************************************************/

  /* create iterators for each vector */
  DoubleVector::iterator it;
  
  /* these vectors should all be the same size  */
  /* so take one and use that to count thru all */
  for(i = 0, it = temp_acceptedIndexes.begin();
      it != temp_acceptedIndexes.end();
      i++, it++)
  {
    acceptedIndexes[i] = (int)*it;
  }
  
  /*   no memory leak here   */
  delete b;
}

