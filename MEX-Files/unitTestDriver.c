// this is a test driver for the
// dspUtils library. necessary to
// ensure its all good before debugging
// the onset detection routines . . .

#include <stdio.h>
#include <math.h>
#include "dspUtils.h"

int main()
{
  int i;
  double j = 2.4;
  double x[10] = {0, .1, .2, .3, .4, .5, .6, .7, .8, .9};
  //  double y[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  //  double *z;
  double *y;

  double B[5] = {0.0003, -0.0013, 0.0018, -0.0013, 0.0003 };
  double A[5] = {1.0000, -3.9890, 5.9673, -3.9677, 0.9894 };
  
  ///////////////////////////////////////////////////////////////////
  // test 5-tap iirFilter
  // B,A, filterLength, *signal, signalLength
  // 
  y = iirFilter(B, A, 5, x, 15);
  
  for(i=0; i < 15; i++)
    {
      printf("output y[%d] = %f\n", i, y[i]);
    }
  
  /*
    
    ///////////////////////////////////////////////////////////////////
    // test convolution z may be bigger than 5 
    z = conv(x, 5, y, 5);
    
    for(i=0; i<5; i++)
    {
    printf("this is z[%d] = %f\n", i, z[i]);
    }
    
    ///////////////////////////////////////////////////////////////////
    // test maximum (rename while you're @ it)
    //  maximum(x, 5, j);
    for(i = 0; i < 5; i++)
    {
    printf("this is x[%d] = %f\n", i, x[i]);
    }  
  */  
  
  ///////////////////////////////////////////////////////////////////  
  // test gaussMF
 
  gaussMF(x, 10, .25, .5);
  for(i = 0; i < 10; i++)
    {
      printf("this is x[%d] = %f\n", i, x[i]);
    }  

  
  /*    
	///////////////////////////////////////////////////////////////////
	// test getMaximumIndex
	i = getMaximumIndex(x, 5);
    
    printf("this is d = %d\n", i);
    
    // test inner_product
    j = inner_product(x, y, 5);
    printf("this is j = %f\n", j);
    
    ///////////////////////////////////////////////////////////////////
    // test returnMinimum 
    printf("the minimum is %d \n", returnMinimum(43,5));
    
    createMAT("x_this_is_test.mat", x,5);
    
  */
  
  return 0;
}
