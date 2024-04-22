/**********************************************************************/
/*                                                                    */   
/*  A collection of useful DSP and Linear Algebra routines and        */
/*  definitions rigged by iroro orife 2000 - 2003                     */
/*                                                                    */   
/**********************************************************************/

#ifndef __DSPUTILS_H__
#define __DSPUTILS_H__

#define PI 3.14159265358979323846264338327950288419

// some essential macros
#ifndef SIGN
#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#endif

#ifndef MIN
#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif


/*
  fast abs/neg/sign for 32bit floats
  
  Type : floating point functions
  References : Posted by tobybear[AT]web[DOT]de
  
  Notes :
  Haven't seen this elsewhere, probably because it is 
  too obvious? Anyway, these functions are intended for 
  32-bit floating point numbers only and should work a 
  bit faster than the regular ones.

  fastabs() gives you the absolute value of a float
  fastneg() gives you the negative number (faster than multiplying with -1)
  fastsgn() gives back +1 for 0 or positive numbers, -1 for negative numbers
  
  Comments are welcome (tobybear[AT]web[DOT]de)
  Cheers
  Toby (www.tobybear.de)

float fastabs(float f)
{int i=((*(int*)&f)&0x7fffffff);return (*(float*)&i);}

float fastneg(float f)
{int i=((*(int*)&f)^0x80000000);return (*(float*)&i);}

int fastsgn(float f)
{return 1+(((*(int*)&f)>>31)<<1);}

*/

/*
  Clipping without branching
  
  Type : Min, max and clip
  References : Posted by Laurent de Soras
  
  Notes :
  It may reduce accuracy for small numbers. I.e. 
  if you clip to [-1; 1], fractional part of the 
  result will be quantized to 23 bits (or more, 
  depending on the bit depth of the temporary results). 
  Thus, 1e-20 will be rounded to 0. The other (positive) 
  side effect is the denormal number elimination.
*/ 

/*
float max (float x, float a)
{
  x -= a;
  x += fabs (x);
  x *= 0.5;
  x += a;
  return (x);
}

float min (float x, float b)
{
  x = b - x;
  x += fabs (x);
  x *= 0.5;
  x = b - x;
  return (x);
}

float clip (float x, float a, float b)
{
  x1 = fabs (x-a);
  x2 = fabs (x-b);
  x = x1 + (a+b);
  x -= x2;
  x *= 0.5;
  return (x);
}
*/


////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * PYTHAG - computes (a^2 + b^2)^1/2 without destructive underflow or 
 * 	    overflow. needs to be tested still w/ double precision data 
 *
 **********************************************************************/
//////////////////////////////////////////////////////////////////////// 
static double at, bt, ct;
#define PYTHAG(a, b) ((at = fabs(a)) > (bt = fabs(b)) ? \
(ct = bt/at, at*sqrt(1.0+ct*ct)): (bt ? (ct = at/bt, bt*sqrt(1.0+ct*ct)): 0.0))


double getMean(double *x, int n);


double* fourthOrderIIRFilter(double *B, 
                             double *A, 
                             double *signal, 
                             int signalLength);

double *conv(double *x, 
             int xlen, 	
             double *y, 
             int ylen);

void maximum(double *x, 
             int xlen, 
             double y);

void gaussMF(double *x, 
             int xlen, 
             double sigma, 
             double c);

int getMaximumIndex(double *x, 
                    int xlen);

int getMaximumAbsIndex(double *x, 
                       int xlen);

double inner_product(double *x, 
                     double *y, 
                     int len);

int returnMinimum(int x, 
                  int y);

double* sixthOrderIIRFilter(double *B, 
                            double *A, 
                            double *signal, 
                            int signalLength);			      									
void svd(double* a[], 
         int m, 
         int n, 
         double w[], 
         double* v[]);
				 
double* psuedoinverse(double *X, 
                      int M,
                      int N,
                      double epsilon);
											
void normalize(double* x, 
               int n);


#endif /* __DSPUTILS_H__ */
