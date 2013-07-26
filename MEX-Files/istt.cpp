
#include <iostream>
#include <string.h>
#include <math.h>


//
// IO io23 fix jan 3, 2001, necessary when calling C methods 
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

		// Default values
		size_t size = mxGetM( prhs[0]);
		size_t hop = size, pad = 0;
		if( nrhs > 1)
		  hop = ( size_t)( *mxGetPr( prhs[1]));
		if( nrhs > 2)
		  pad = ( size_t)( *mxGetPr( prhs[2]));
		int tp = 0;
		if( nrhs > 4)
		  tp = ( size_t)( *mxGetPr( prhs[4]));

		// Select transform type
		void (*tw)( int n, double *);
		void (*tr)( int n, double *, double *);
		double nrm;
		bool cc = false;
		switch( tp){
			case 0:
				cc = true;
				size = (size-1)*2;
			case 1:
				tw = &rffti;
				tr = &rfftb;
				nrm = size;
			break;
			case 2:
				tw = &costi;
				tr = &cost;
				nrm = 2*(size-1);
			break;
			case 3:
				tw = &sinti;
				tr = &sint;
				nrm = 2*(size+1);
			break;
			case 4:
				tw = &cosqi;
				tr = &cosqb;
				nrm = size;
			break;
			case 5:
				tw = &sinqi;
				tr = &sinqb;
				nrm = size;
			break;
		}

		// Get window
		double *win = NULL;
		if( nrhs > 3){
			win = mxGetPr( prhs[3]);
			if( mxGetN( prhs[3])*mxGetM( prhs[3]) != size-pad)
				mexErrMsgTxt("Window has wrong size");
		}

		// Create output
		size_t m = mxGetM( prhs[0]), n = mxGetN( prhs[0]);
		size_t l = (n+1)*hop+size;
		if( nlhs > 0)
		  mxFree( plhs[0]);
		plhs[0] = mxCreateDoubleMatrix( 1, l, mxREAL);

		// Set up pointers
		double *x = mxGetPr( plhs[0]);
		double *fi, *fr = mxGetPr( prhs[0]);
		if( cc){
			fi = mxGetPi( prhs[0]);
			if( fi == NULL)
				mexErrMsgTxt("No imaginary part in input");
		}

		// Setup twiddles
		double *w  = new double[3*size+15];
		if( w == NULL)
			mexErrMsgTxt("Memory allocation error");
		(*tw)( size, w);

		// Temporary buffer
		double *f  = new double[size];
		if( f == NULL)
			mexErrMsgTxt("Memory allocation error");

		// Zero out output
		size_t i;
		for( i = 0 ; i != l ; i++)
			x[i] = 0.;

		// Perform ISTFT
		for( i = 0 ; i < n ; i++){

			// Unpack data
			if( cc){
				f[0] = *fr++;
				*fi++;
				for( size_t k = 1 ; k < size-1 ; k+=2){
					f[k] = *fr++;
					f[k+1] = *fi++;
				}
				f[size-1] = *fr++;
				*fi++;
			}else
				for( size_t k = 0 ; k != size ; k++)
					f[k] = *fr++;

			// Do inverse transform
			(*tr)( size, f, w);

			// Copy over data
			if( win == NULL)
				for( size_t k = 0 ; k != size-pad ; k++)
					x[i*hop+k] += f[k];
			else
				for( size_t k = 0 ; k != size-pad ; k++)
					x[i*hop+k] += f[k] * win[k];
		}

		// Free up taken memory
		delete [] f;
		delete [] w;

		// Scale
		for( i = 0 ; i != l ; i++){
			x[i] /= nrm;
		}

/*		// Fix first and last overlaps
		for( i = 0 ; i != hop ; i++){
			x[i] *= (size-pad)/hop;
			x[l-i-1] *= (size-pad)/hop;
		}*/

  }else
		// Wrong arguments?  Exit!
		mexErrMsgTxt("Wrong number of arguments");
}
