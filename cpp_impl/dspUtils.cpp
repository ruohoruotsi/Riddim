//////////////////////////////////////////////////////////////////////
/*********************************************************************
 * 
 * getMean - returns the mean of a signal
 *
 * dspUtils.c - a collection of common and useful DSP and Linear 
 *              Algebra routines.
 *              
 * fourthOrderIIRFilter(B,A, ...)  - Computational function that takes two 
 *			         vectors of coefficients B & A and filters 
 * 			         them with an 4th order IIR filter.
 * 
 * conv(x,y)  - canonical convolution of two vectors x, y
 *		(see Oppenheim & Schafer)
 *
 * maximum(x,y) - replaces in each element of the input vector x, 
 *      	  with the maximum of x[i] and a scalar y 
 * 
 * gaussMF(x,sigma,C) - gaussian membership function, creates in x,
 * 			a gaussian function with properties sigma,C
 *
 * getMaximumIndex(x) - returns the index of the element of x with 
 * 			the largest value		
 * 
 * inner_product(x,y) - performs the inner product of x and y
 *
 * returnMinimum(x,y) - returns the minium of the x and y
 *
 * sixthOrderIIRFilter(B,A, ...) - 7 tap IIR Filter
 *
 * pythag(a,b) - computes (a^2 + b^2)^1/2 without destructive 
 *		 underflow or overflow
 *
 * Singular Value Decomposition (SVD) - matrix decomposition routine 
 *                                      taken from Numerical Recipes
 *
 * psuedoinverse(A) - uses the SVD to calculate the pseduoinverse
 *		      of a matrix A
 *
 * normalize(x) - normalize a vector x 
 *
 *
 ********************************************************************
 *
 * slapped together by iroro orife (iroro@bigfoot.com),  2000 - 2002
 *
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dspUtils.h"

////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * getMean - returns the mean of a signal
 * 
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
double getMean(double *x, int n)
{
  double m = 0;
  for (int i = 0; i < n; i++)
  {
    m += x[i];
  }
  m = m / (double) n;
  return m;
}


////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * fourthOrderIIRFilter - The filter is a "Direct Form II Transposed"
 * implementation of the standard difference equation:
 *
 * a(1)*y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 * - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
double* fourthOrderIIRFilter(double *B, 
                             double *A, 
                             double *signal, 
                             int signalLength)
{
  // implementation of a fourth order, 5 tap iir filter
  // x is the temporary input signal 
  // y is the temporary vector for the filtering
  // output is the filtered vector
  
  int i;
  int filterLength = 5;
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
    poles =           A[1]*y[0] + A[2]*y[1] + A[3]*y[2] + A[4]*y[3];
      
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
 * conv - linear convolution
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
/**********************************************************************
 * maximum - this method is equivalent to the MATLAB
 *           function 'max' that takes a vector x and
 *	     a value y and calculates in place for 
 *    	     each element of x the maximum between that 
 *           element and y
 *********************************************************************/
///////////////////////////////////////////////////////////////////////
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
 * gaussMF - an implementation of the gaussian membership function
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
/***********************************************************************
 *
 * getMaximumIndex - returns the index of the element of x with the 
 * 		     largest value		
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
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
/***********************************************************************
 *
 * getMaximumAbsIndex - returns the index of the element of x with the 
 * 		        largest absolute value		
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
int getMaximumAbsIndex(double *x, 
                       int xlen)
{
  int i = 0;
  int maxid = 0;
  double maxvalue = 0;
  
  for(i = 0; i < xlen; i++)
  {
    if(maxvalue < fabs(x[i]))
    {
      maxvalue = fabs(x[i]);
      maxid = i;
    }
  }
  return maxid;
}


////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * inner_product - performs the inner product of x and y = Summation of 
 * 		   (x[i]*y[i]) for i element of [0, N-1] N = xlen
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
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
/***********************************************************************
 *
 * returnMinimum - returns the minium of the two integral arguments
 *
 **********************************************************************/
////////////////////////////////////////////////////////////////////////
int returnMinimum(int x, 
                  int y)
{
  if(x < y) return x;
  return y;
}



////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * sixthOrderIIRFilter - just like iirFilter but has more taps, 6th
 * order means 6+1 size coefficient vectors B,A are needed!
 *
 **********************************************************************/
//////////////////////////////////////////////////////////////////////// 
double* sixthOrderIIRFilter(double *B, 
                            double *A, 
                            double *signal, 
                            int signalLength)
{
  // implementation of a 6-tap iir filter
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
    poles =             A[1]*y[0] + A[2]*y[1] + A[3]*y[2] + A[4]*y[3] + A[5]*y[4] + A[6]*y[5];
      
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



////////////////////////////////////////////////////////////////////////
/***********************************************************************
 *
 * svd - Given a matrix a[m][n], this routine computes its singular 
 * 	 value decomposition, A = U*W*V^{T}.  The matrix U replaces a 
 *	 on output. The diagonal matrix of singular values W is output 
 * 	 as a vector w[n].  The matrix V (not the transpose V^{T}) is 
 *	 output as v[n][n]. m must be greater or equal to n;  if it is 
 *	 smaller, then a should be filled up to square with zero rows. 
 *	 Taken from Numerical Recipes.
 *
 **********************************************************************/
//////////////////////////////////////////////////////////////////////// 
void svd(double* a[], 
         int m, 
         int n, 
         double w[], 
         double* v[])
{
  int flag, i, its, j, jj, k, l, nm;
  double c, f, h, s, x, y, z;
  double anorm = 0.0, g = 0.0, scale = 0.0;
  
  // test kosherness of the dimensions of the matrix
  if(m < n)
  {
    printf("SVD: M < N!!!:  Matrix A must be augmented with extra rows of zeros!!");
    return;
  }
  
  double* rv1 = new double [n];
  
  /* Householder reduction to bidiagonal form.			*/
  for (i = 0; i < n; i++)
  {
    l = i + 1;
    rv1[i] = scale*g;
    g = s = scale = 0.0;
    if (i < m)
    {
      for (k = i; k < m; k++)
        scale += fabs(a[k][i]);
      if (scale)
      {
        for (k = i; k < m; k++)
        {
          a[k][i] /= scale;
          s += a[k][i]*a[k][i];
        };
        f = a[i][i];
        g = -SIGN(sqrt(s), f);
        h = f*g - s;
        a[i][i] = f - g;
        if (i != n - 1)
        {
          for (j = l; j < n; j++)
          {
            for (s  = 0.0, k = i; k < m; k++)
              s += a[k][i]*a[k][j];
            f = s/h;
            for ( k = i; k < m; k++)
              a[k][j] += f*a[k][i];
          };
        };
        for (k = i; k < m; k++)
          a[k][i] *= scale;
      };
    };
    w[i] = scale*g;
    g = s= scale = 0.0;
    if (i < m && i != n - 1)
    {
      for (k = l; k < n; k++)
        scale += fabs(a[i][k]);
      if (scale)
      {
        for (k = l; k < n; k++)
        {
          a[i][k] /= scale;
          s += a[i][k]*a[i][k];
        };
        f = a[i][l];
        g = -SIGN(sqrt(s), f);
        h = f*g - s;
        a[i][l] = f - g;
        for (k = l; k < n; k++)
          rv1[k] = a[i][k]/h;
        if (i != m - 1)
        {
          for (j = l; j < m; j++)
          {
            for (s = 0.0, k = l; k < n; k++)
              s += a[j][k]*a[i][k];
            for (k = l; k < n; k++)
              a[j][k] += s*rv1[k];
          };
        };
        for (k = l; k < n; k++)
          a[i][k] *= scale;
      };
    };
    anorm = MAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
  };
  
  /* Accumulation of right-hand transformations.			*/
  for (i = n - 1; 0 <= i; i--)
  {
    if (i < n - 1)
    {
      if (g)
      {
        for (j = l; j < n; j++)
          v[j][i] = (a[i][j]/a[i][l])/g;

        /* Double division to avoid possible underflow:	*/
        for (j = l; j < n; j++)
        {
          for (s = 0.0, k = l; k < n; k++)
            s += a[i][k]*v[k][j];
          for (k = l; k < n; k++)
            v[k][j] += s*v[k][i];
        };
      };
      for (j = l; j < n; j++)
        v[i][j] = v[j][i] = 0.0;
    };
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;
  };
  
  /* Accumulation of left-hand transformations. */
  for (i = n - 1; 0 <= i; i--)
  {
    l = i + 1;
    g = w[i];
    if (i < n - 1)
      for (j = l; j < n; j++)
        a[i][j] = 0.0;
    if (g)
    {
      g = 1.0/g;
      if (i != n - 1)
      {
        for (j = l; j < n; j++)
        {
          for (s = 0.0, k = l; k < m; k++)
            s += a[k][i]*a[k][j];
          f = (s/a[i][i])*g;
          for (k = i; k < m; k++)
            a[k][j] += f*a[k][i];
        };
      };
      for (j = i; j < m; j++)
        a[j][i] *= g;
    }
    else
      for (j = i; j < m; j++)
        a[j][i] = 0.0;
    ++a[i][i];
  };

  /* Diagonalization of the bidiagonal form.				*/
  for (k = n - 1; 0 <= k; k--)	/* Loop over singular values.	*/
  {
    for (its = 0; its < 30; its++)	/* Loop over allowed iterations.*/
    {
      flag = 1;
      for (l = k; 0 <= l; l--)	/* Test for splitting:		*/
      {
        nm = l - 1;		/* Note that rv1[0] is always zero.*/
        if (fabs(rv1[l]) + anorm == anorm)
        {
          flag = 0;
          break;
        };
        if (fabs(w[nm]) + anorm == anorm)
          break;
      };
      if (flag)
      {
        c = 0.0;		/* Cancellation of rv1[l], if l>0:*/
        s = 1.0;
        for (i = l; i <= k; i++) {
          f = s*rv1[i];
          if (fabs(f) + anorm != anorm)
          {
            g = w[i];
            h = PYTHAG(f, g);
            w[i] = h;
            h = 1.0/h;
            c = g*h;
            s = (-f*h);
            for (j = 0; j < m; j++)
            {
              y = a[j][nm];
              z = a[j][i];
              a[j][nm] = y*c + z*s;
              a[j][i]  = z*c - y*s;
            };
          };
        };
      };
      z = w[k];
      if (l == k)		/* Convergence.				*/
      {
        if (z < 0.0)	/* Singular value is made non-negative.	*/
        {
          w[k] = -z;
          for (j = 0; j < n; j++)
            v[j][k] = (-v[j][k]);
        };
        break;
      };

      // test kosherness
      if (its == 29)
      {
        printf("No convergence in 30 SVD iterations.");
        return;
      }
      
      x = w[l];		/* Shift from bottom 2-by-2 minor.	*/
      nm = k - 1;
      y = w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z)*(y + z) + (g - h)*(g + h))/(2.0*h*y);
      g = PYTHAG(f, 1.0);
      f = ((x - z)*(x + z) + h*((y/(f + SIGN(g, f))) - h))/x;

      /* Next QR transformation:					*/
      c = s = 1.0;
      for (j = l; j <= nm; j++)
      {
        i = j + 1;
        g = rv1[i];
        y = w[i];
        h = s*g;
        g = c*g;
        z = PYTHAG(f, h);
        rv1[j] = z;
        c = f/z;
        s = h/z;
        f = x*c + g*s;
        g = g*c - x*s;
        h = y*s;
        y = y*c;
        for (jj = 0; jj < n;  jj++)
        {
          x = v[jj][j];
          z = v[jj][i];
          v[jj][j] = x*c + z*s;
          v[jj][i] = z*c - x*s;
        };
        z = PYTHAG(f, h);
        w[j] = z;	/* Rotation can be arbitrary if z = 0.	*/
        if (z)
        {
          z = 1.0/z;
          c = f*z;
          s = h*z;
        };
        f = (c*g) + (s*y);
        x = (c*y) - (s*g);
        for (jj = 0; jj < m; jj++)
        {
          y = a[jj][j];
          z = a[jj][i];
          a[jj][j] = y*c + z*s;
          a[jj][i] = z*c - y*s;
        };
      };
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = x;
    };
  };
  delete [] rv1;
}


//////////////////////////////////////////////////////////////////////// 
/***********************************************************************							
 * psuedoinverse - uses the SVD to decompose, then re-arrange the 
 *	    	   composite matrices, see below props of a psuedoinv.
 *                 the routine takes a matrix X as a "flattened" array
 *                 of size M*N, need to make sure X is a tall or square
 *	   	   matrix of M <= N, the returned A' is an N*M matrix 
 *		   that is the transpose of A'
 *
 *   X = PINV(A) produces a matrix X of the same dimensions
 *   as A' so that A*X*A = A, X*A*X = X and A*X and X*A
 *   are Hermitian. The computation is based on SVD(A) and any
 *   singular values less than a tolerance are treated as zero.
 *   The default tolerance is MAX(SIZE(A)) * NORM(A) * EPS.
 *								 
 *  this implementation is the bastard child of iroro and a number of
 *  freeware implementations available on the web 				
 *			   							
 **********************************************************************/ 
//////////////////////////////////////////////////////////////////////// 
double* psuedoinverse(double *X, 
                      int M,
                      int N,
                      double epsilon)
{	
  double*  a = new double [N*M];
  double* uu = new double [M*N];
  double* vv = new double [N*N];
  double*  w = new double [N];
  double** u = new double* [M];
  double** v = new double* [N];
  
  
  for (int i = 0; i < M; i++)
    u[i] = &(uu[N*i]);
  for (int j = 0; j < N; j++)
    v[j] = &(vv[N*j]);
  {
    for (int i = 0; i < M; i++)
      for (int j = 0; j < N; j++)
        u[i][j] = X[i*N+j];  // prepare input matrix for SVD
  }

  svd(u, M, N, w, v);	  // Singular value decomposition.
  double wmax = 0.0;	  // Maximum singular value.
  {
    for (int j = 0; j < N; j++)
      if (w[j] > wmax)
        wmax = w[j];
  }
  double wmin = wmax*epsilon;
  for (int k = 0; k < N; k++)
    if (w[k] < wmin)
      w[k] = 0.0;
    else
      w[k] = 1.0/w[k];
  {
    for (int i = 0; i < N; i++)
      for (int j = 0; j < M; j++) {
        a[M*i+j] = 0.0;
        for (int k = 0; k < N; k++)
          a[M*i+j] += v[i][k]*w[k]*u[j][k];
      };
  }
  delete [] w;
  delete [] u;
  delete [] v;
  delete [] uu;
  delete [] vv;

  return a;
  // return doubleMatrix(N, M, a);
}



//////////////////////////////////////////////////////////////////////// 
/***********************************************************************							
 * normalize -  takes a vector and puts its values proportionally 
 *		between [-1,1], by dividing every element by the 
 *		maximum value in the vector
 *			   							
 **********************************************************************/ 
//////////////////////////////////////////////////////////////////////// 
void normalize(double* x, int n)
{
  double maxValue = x[getMaximumAbsIndex(x,n)];
	
  for(int i = 0; i < n; i++)
  {
    x[i] = x[i]/(maxValue);
  }
}



