/*
  Short Time Fourier Transform (forward and backward transforms)
  uses the Real Fast Fourier Transform in the West (RFFTW)
  
  (C) iroro orife, 2001, All Rights Reserved
  
*/


#include <string.h>
#include <math.h>
#include "stft.h"

// real fftw 
#include "rfftw.h"

// size_t == unsigned int ...

/* forward short time forward transform */
void stft(double* x, 
          long    inputSizeInSamples,
          int     fftSize, 
          int     hopSize, 
          double* window, 
          double* fi, 
          double* fr)
{
  // initialize local variables ...
  size_t l = inputSizeInSamples;  // mono vector * 1
  rfftw_plan p = rfftw_create_plan(fftSize, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
  
  fftw_real* f  = new fftw_real[fftSize];
  fftw_real* outf = new fftw_real[fftSize]; 
  
  size_t j = 0; // index into the output frequency arrays 
  size_t k = 0;

  // do windowed transform @ every hop, until you 
  // reach the end of the vector ...
  for(size_t i = 0; i < l; i += hopSize)
  {
    // Zero buffer everytime we go in ...
    memset(    f, 0, fftSize*sizeof( double));
    memset( outf, 0, fftSize*sizeof(double));
    
    // Copy over data into the temporary buffer
    if( i+fftSize < l) // the evaluates to true if we're not hanging off the end
    { 							
      for( k = 0; k != fftSize; k++) // window data
      {
        f[k] = x[i + k] * window[k];
      }      
    }
    else // we're hanging off the edge of x here now
    {    // this logic executes on the last windows of x
    
      for( k = i; k != l; k++) // window data
      {
        f[k-i] = x[k] * window[k-i];
      }
    }   // end hanging off end if-else
    
    
    // Do the transform: size, temp buffer, window
    rfftw_one(p, f, outf); 

    // Pack data that we get back in "f" 
    //
    fr[j] = outf[0]; // catch the first ones
    fi[j] = 0;   
    
    // complex data is interleaved, unpack      
    for( k = 1; k < (fftSize+1)/2 ; k++)
    {
      fr[j + k] = outf[k];   
      fi[j + k] = outf[fftSize - k]; 
    }
    
    // fftSize ~must~ be even
    fr[j + k] = outf[k];     //  fftSize/2
    fi[j + k] = 0;       
    j += fftSize/2 + 1;   // jump ahead to the beginning of the next row
    
  }  // end for (traversing x, hopping each window)
  
  // Free heap buffers
  delete [] f;
  delete [] outf;
  rfftw_destroy_plan(p);
}



/* inverse short time fourier transform */
void istft (double* x, 
            long    numberOfTimeSlices,
            int     halfComplexSize, 
            int     hopSize,  
            double* window, 
            double* fi, 
            double* fr)
{
  // initialize local variables
  size_t n = numberOfTimeSlices;       // n == number of time slices in spectrogram
  int fftSize = (halfComplexSize-1)*2; // m == halfcomplex size = fftSize/2 + 1;
  
  size_t l = (n+1)*hopSize+fftSize;		 // the reconstructed time series vector
  rfftw_plan p = rfftw_create_plan(fftSize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
	
    
  // Temporary buffer
  fftw_real *f  = new fftw_real[fftSize];
  fftw_real *outf = new fftw_real[fftSize];
  
  size_t i = 0;
  size_t j = 0; // index into the output frequency arrays ...
  size_t k = 0;
	
  // Perform ISTFT
  for( i = 0 ; i < n ; i++)
  {
    // Unpack complex data into one vector ... 
    f[0] = fr[j];  
     
    for( k = 1;  k < (fftSize+1)/2; k++)
    {
      f[k]   = fr[j + k];
      f[fftSize - k] = fi[j + k]; 
    }
    
    // fftSize ~must~ be even
    f[k] = fr[j + k];  
    j += fftSize/2 + 1;
     
     
    // Do inverse transform
    rfftw_one(p, f, outf); 
    	
    // re-patch together time series ...	
    for( k = 0 ; k < fftSize; k++)
    {
      x[i*hopSize + k] += outf[k] * window[k];
    }
    
  } // end for loop
	
  
  // Free up taken memory
  delete [] f;
  delete [] outf;
  rfftw_destroy_plan(p);
	
  // Scale the output
  for( i = 0 ; i != l ; i++)
  {
    x[i] /= fftSize;
  }	
	
  /*		
  // Fix first and last overlaps
  for( i = 0 ; i != hop ; i++){
  x[i] *= (size-pad)/hop;
  x[l-i-1] *= (size-pad)/hop;
  }
  */
}
