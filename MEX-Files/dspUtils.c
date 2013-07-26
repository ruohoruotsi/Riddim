/*
  //////////////////////////////////////////////////////////////////////
  dspUtils.c
  
  various dsp utility methods 
  
  iirFilter - Computational function that 
  takes two vectors of coefficients B & A and 
  filters them with an 5 tap IIR filter.
  
  convolution - 
  
  gaussian - 
  
  max - returns the maximum of x[i] and y for all i [0,xlen]
  
  hacked together by iroro orife 14-12-2000
  
  //////////////////////////////////////////////////////////////////////
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "mat.h"

////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * The filter is a "Direct Form II Transposed"
 * implementation of the standard difference equation:
 *
 * a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 * - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
double* iirFilter(double *B, 
		  double *A, 
		  int filterLength, 
		  double *signal, 
		  int signalLength)
{
  // implementation of a 5-tap iir filter
  // x is the temporary input signal 
  // y is the temporary vector for the filtering
  // output is the filtered vector
  
  int i;
  double *x, *y, *output;
  double zeros = 0;
  double poles = 0;
  
  // if a(1) is not equal to 1, normalize coefficients by A[1]
  if(A[0] != 1)
    {
      for(i=0; i < filterLength; i++)
	{
	  A[i] = A[i]/A[0];
	  B[i] = B[i]/A[0];
	}
    }
  
  // allocate memory and initialize vectors
  x = (double*)calloc((filterLength-1), sizeof(double));
  y = (double*)calloc((filterLength-1), sizeof(double));
  output = (double*)calloc((signalLength), sizeof(double));

  // still unsure whether the zero filling will work for 
  // floating point numbers . . .  
  for(i =0; i < filterLength-1; i++)
    y[i] = 0;
  
  for(i =0; i < filterLength-1; i++)
    x[i] = 0;
  
  for(i =0; i < signalLength; i++)
    output[i] = 0;

  /* 
     The filter is a "Direct Form II Transposed"
     implementation of the standard difference equation:
     
     a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
     - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
  */
  
  for(i = 0; i < signalLength; i++)
    {
      zeros = 
	B[0]*signal[i] +  B[1]*x[0] + B[2]*x[1] + B[3]*x[2] + B[4]*x[3];
      poles =             A[1]*y[0] + A[2]*y[1] + A[3]*y[2] + A[4]*y[3];
      
      output[i] = zeros - poles;
      
      // update x history
      x[3] = x[2];
      x[2] = x[1];
      x[1] = x[0];
      x[0] = signal[i];
      
      // update y history 
      y[3] = y[2];
      y[2] = y[1];
      y[1] = y[0];
      y[0] = output[i];
    }
  
  free(x);
  free(y);
  
  return output;
}


////////////////////////////////////////////////////////////////////////
/***********************************************************************
 * linear convolution
 * 
 * Call: x is a vector of xlen elements
 *       y is a vector of ylen elements
 *       z is a vector to hold xlen+ylen-1 elements
 *
 * Result of the convolution of x and y is returned 
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
double *conv(double *x, 
	     int xlen, 
	     double *y, 
	     int ylen)
{
  int i;      //index into z
  int j;      //index into y
  int k;      //index into x
  double *z;
  int zlen;   //length of z
  
  zlen = xlen + ylen - 1;
  z = (double *)malloc(zlen*sizeof(double));
  
  for(i = 0; i < zlen; i++)   // for each value in z
    {
      z[i] = 0.0;             
      for(j = 0, k = i; j < ylen; j++, k--)
	{
	  if(k >= xlen) continue; //trailing edge, region 3
	  z[i] += x[k] * y[j];    //overlap, region 2
	  if(k == 0) break;       //leading edge, region 1
	}
    }
  
  // chop off the trailing (ylen - 1), so that the input vector
  // is the same size as the return vector
  //  for(i = 0; i < xlen; i++)
  //  x[i] = z[i];
  
  return z;
}

////////////////////////////////////////////////////////////////////////
// this method is equivalent to the MATLAB
// function 'max' that takes a vector x and
// a value y and calculates in place for 
// each element of x the maximum between that 
// element and y
//
void maximum(double *x, 
	     int xlen, 
	     double y)
{
  int i;
  
  for(i = 0; i < xlen; i++)
  {
    if(x[i] < y)
    {
      x[i] = y;
    }
  }
}


////////////////////////////////////////////////////////////////////////
/***********************************************************************
 * an implementation of the gaussian membership function
 *
 * returns a vector with the gaussian membership function
 * evaluated @ each indice, sigma and C determine the shape 
 * and position of the membership function 
 * return value = exp( (-(x[i] - C))^2/(2*sigma^2) )
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////

void gaussMF(double *x, 
	     int xlen, 
	     double sigma, 
	     double C)
{
  int i;
  for(i = 0; i < xlen; i++)
    {
      x[i] = exp((-1)*pow(-(x[i] - C), 2)/(2*pow(sigma,2)));
    }
}


////////////////////////////////////////////////////////////////////////
//
// this method returns the index of the element of x with the 
// largest value		
//
int getMaximumIndex(double *x, 
		    int xlen)
{
  int i = 0;
  int maxid = 0;
  double maxvalue = 0;
  
  for(i = 0; i < xlen; i++)
  {
      if(maxvalue < x[i])
	{
	  maxvalue = x[i];
	  maxid = i;
	}
    }
  return maxid;
}


////////////////////////////////////////////////////////////////////////
//
// performs the inner product of x and y = Summation of (x[i]*y[i] for
// i element of [0, N-1] N = xlen
//
double inner_product(double *x, 
		     double *y, 
		     int len)
{
  int i;
  double sum = 0;
  
  for(i = 0; i < len; i++)
    sum += x[i]*y[i];
  return sum;
}


////////////////////////////////////////////////////////////////////////
//
// returns the minium of the two arguments - integral type
//
int returnMinimum(int x, 
		  int y)
{
  if(x < y) return x;
  return y;
}


////////////////////////////////////////////////////////////////////////
//
// create a MAT file from a double passed in
//
int createMAT(const char *file, 
	      double *data, 
	      int datalen) 
{
  MATFile *pmat;
  mxArray *pa1; 
  
  printf("Creating a .MAT file %s...\n", file);
  pmat = matOpen(file, "w");
  if (pmat == NULL) 
    {
      printf("Error creating file %s\n", file);
      printf("(do you have write permission in this directory?)\n");
      return(1);
    }
  
  pa1 = mxCreateDoubleMatrix(1,datalen,mxREAL);
  mxSetName(pa1, "iroro_debug_variable");

  memcpy((char *)(mxGetPr(pa1)),(char *)data, 1*datalen*sizeof(double));
  matPutArray(pmat, pa1);

  // clean up
  mxDestroyArray(pa1);

  
  if (matClose(pmat) != 0) {
    printf("Error closing file %s\n",file);
    return(1);
  }

  printf("Done\n");
  return(0);
}


////////////////////////////////////////////////////////////////////////
// 
double* seventhOrderIIRFilter(double *B, 
			      double *A, 
			      double *signal, 
			      int signalLength)
{
  // implementation of a 7-tap iir filter
  // x is the temporary input signal 
  // y is the temporary vector for the filtering
  // output is the filtered vector
  
  int i; 
  int filterLength = 7;
  double *x, *y, *output;
  double zeros = 0;
  double poles = 0;
  
  // if a(1) is not equal to 1, normalize coefficients by A[1]
  if(A[0] != 1)
    {
      for(i=0; i < filterLength; i++)
	{
	  A[i] = A[i]/A[0];
	  B[i] = B[i]/A[0];
	}
    }
  
  // allocate memory and initialize vectors
  x = (double*)calloc((filterLength-1), sizeof(double));
  y = (double*)calloc((filterLength-1), sizeof(double));
  output = (double*)calloc((signalLength), sizeof(double));
  
  // still unsure whether the zero filling will work for 
  // floating point numbers . . .  
  for(i =0; i < filterLength-1; i++)
    y[i] = 0;
  
  for(i =0; i < filterLength-1; i++)
    x[i] = 0;
  
  for(i =0; i < signalLength; i++)
    output[i] = 0;
  
  /* 
     The filter is a "Direct Form II Transposed"
     implementation of the standard difference equation:
     
     a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
     - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
  */
  
  for(i = 0; i < signalLength; i++)
    {
      zeros = 
	B[0]*signal[i] +  B[1]*x[0] + B[2]*x[1] + B[3]*x[2] + B[4]*x[3] + B[5]*x[4] + B[6]*x[5];
	poles =           A[1]*y[0] + A[2]*y[1] + A[3]*y[2] + A[4]*y[3] + A[5]*y[4] + A[6]*y[5];
      
      output[i] = zeros - poles;
      
      // update x history
      x[5] = x[4];
      x[4] = x[3];
      x[3] = x[2];
      x[2] = x[1];
      x[1] = x[0];
      x[0] = signal[i];
      
      // update y history 
      y[5] = y[4];
      y[4] = y[3];
      y[3] = y[2];
      y[2] = y[1];
      y[1] = y[0];
      y[0] = output[i];
    }
  
  free(x);
  free(y);
  
  return output;
}

