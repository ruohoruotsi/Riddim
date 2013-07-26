

#include <iostream>
#include <math.h>
#include <string.h>

//
// io23 fix jan 3, 2001, necessary when calling C methods 
// from a C++ context!!
extern "C" {
 #include "mex.h"
 #include "telafft.h"
}


using namespace std;

void mexFunction( int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs)
{
  // Check in-out syntax
  if( nrhs > 0){
    if( mxGetN( prhs[0]) != 1 && mxGetM( prhs[0]) != 1)
      mexErrMsgTxt("Can only transform vector inputs");
    
    // Default values
    size_t size = 128;
    if( nrhs > 1)
      size = ( size_t)( *mxGetPr( prhs[1]));	 
    size_t hop = size, pad = 0;
    if( nrhs > 2)
      hop = ( size_t)( *mxGetPr( prhs[2]));
    if( nrhs > 3)
      pad = ( size_t)( *mxGetPr( prhs[3]));
    int sp = size+pad;
    double *win = NULL;

    if( nrhs > 4){
      win = mxGetPr( prhs[4]);
      if( mxGetN( prhs[4])*mxGetM( prhs[4]) != size)
	mexErrMsgTxt("Window has wrong size");
    }
    int tp = 0;

    if( nrhs > 5)
      tp = ( size_t)( *mxGetPr( prhs[5]));
    
    // Select transform type
    void (*tw)( int n, double *);
    void (*tr)( int n, double *, double *);
    bool cc = false;
    switch( tp){
    case 0:
      cc = true;
    case 1:
      tw = &rffti;
      tr = &rfftf;
      break;
    case 2:
      tw = &costi;
      tr = &cost;
      break;
    case 3:
      tw = &sinti;
      tr = &sint;
      break;
    case 4:
      tw = &cosqi;
      tr = &cosqf;
      break;
    case 5:
      tw = &sinqi;
      tr = &sinqf;
      break;
    default:
      mexErrMsgTxt("Unknown transform type");
    }
    
    // Create output
    size_t l = mxGetM( prhs[0]) * mxGetN( prhs[0]);
    if( nlhs > 0)
      mxFree( plhs[0]);
    if( cc)
      plhs[0] = mxCreateDoubleMatrix( sp/2+1, ceil( double( l)/hop), mxCOMPLEX);
    else
      plhs[0] = mxCreateDoubleMatrix( sp, ceil( double( l)/hop), mxREAL);
    
    // Get data pointers
    double *x = mxGetPr( prhs[0]);
    double *fi, *fr = mxGetPr( plhs[0]);
    if( cc)
      fi = mxGetPi( plhs[0]);
    
    // Make twiddles
    double *w = new double[3*sp+15];
    if( w == NULL)
      mexErrMsgTxt("Memory allocation error");
    (*tw)( sp, w);
    
    // Temporary buffer
    double *f  = new double[sp];
    if( f == NULL)
      mexErrMsgTxt("Memory allocation error");
    
    for( size_t i = 0 ; i < l ; i+=hop){
      
      // Zero buffer
      memset( f, 0, sp*sizeof( double));
      
      // Copy over data
      if( i+size < l)
	if( win == NULL)
	  memcpy( f, x+i, sp*sizeof( double));
	else
	  for( size_t k = 0 ; k != size ; k++)
	    f[k] = x[k+i] * win[k];
      else
	if( win == NULL)
	  for( size_t k = i ; k != l ; k++)
	    f[k-i] = x[k];
	else
	  for( size_t k = i ; k != l ; k++)
	    f[k-i] = x[k] * win[k-i];
      
      // Do the transform
      (*tr)( sp, f, w);					
      
      // Pack data
      if( cc){
	*fr++ = f[0];
	*fi++ = 0;

	for( size_t k = 1 ; k < sp-2 ; k+=2){
	  *fr++ = f[k];
	  *fi++ = f[k+1];
	}
	*fr++ = f[sp-1];
	*fi++ = 0;
      }else
	for( size_t k = 0 ; k != sp ; k++)
	  *fr++ = f[k];
    }
    
    // Free buffers
    delete [] w;
    delete [] f;
    
  }else
    // Wrong arguments?  Exit!
    mexErrMsgTxt("Wrong number of arguments");
}
