/*
 *
 * pinv.c
 *
 *
 * included are helper matrix and svd (i know redundant) methods
 * iroro orife needs to clean this up ... realtime performance
 * liability
 *
 *
 * Modified from pinvT.c by Ben S. Feinstein 11/17/99
 * (c) 1997 Robert Oostenveld, available under GNU licence
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pinv.h"

// #define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#define NR_END 1
#define FREE_ARG char*

float pythag(float a, float b)
/* Computes (a^2 + b^2)^1/2 without destructive underflow or overflow */
{
    float absa,absb;
    absa=fabs(a);
    absb=fabs(b);
    if (absa > absb) return absa*sqrt(1.0+SQR(absb/absa));
    else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}


void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
    fprintf(stderr,"Numerical Recipes run-time error...\n");
    fprintf(stderr,"%s\n",error_text);
    fprintf(stderr,"...now exiting to system...\n");
    exit(1);
}

float *vector(long nl, long nh)
/* allocate a float vector with subscript range v[n;..nh] */
{
    float *v;

    v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
    if (!v) nrerror("allocation failure in vector()");
    return v-nl+NR_END;
}

float **matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript m[nrl..nrh][nch..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
    float **m;

    /* allocate pointers to rows */
    m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));

    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;

    /* allocate rows and set pointers to them */
    m[nrl]=(float*) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for (i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    /* return pointer to array of pointers to rows */
    return m;
}

void free_vector(float *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
    free((FREE_ARG) (v+nl-NR_END));
}

void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
    free((FREE_ARG) (m[nrl]+ncl-NR_END));
    free((FREE_ARG) (m+nrl-NR_END));
}




/*****************************************************************************
 *
 * pinv - pseudoinverse of a square or nonsquare matrix
 *        Multiplication of pinv(A) with A gives the identity matrix.
 *        This function replaces the matrix A with the *transverse* of the 
 *        pseudoinverse matrix.
 *
 ****************************************************************************/
int pinv( float **A, 	  /* matrix[1..Nrows][1..Ncolumns] */
          int Nrows, 	  /* number of rows    */
          int Ncolumns	/* number of columns */
          )    
{
  float **U, **V, *s, *pta, *ptb, max;
  
  // float **U, **V, *s, *pta, max;
  int i, j, k;
  
  // fprintf(stderr, "pinv: Nrows    = %i\n", Nrows);
  // fprintf(stderr, "pinv: Ncolumns = %i\n", Ncolumns);
  // DEBUG("pinvT: Nrows    = %i\n", Nrows);
  // DEBUG("pinvT: Ncolumns = %i\n", Ncolumns);
  
  if (A==NULL || Nrows < 1 || Ncolumns <1) {
    fprintf(stderr, "data invalid, exiting...");
    //	exit(1);
    // msa_error(ERR_DATA);
  }
  
  
  U = matrix(1,Nrows,1,Ncolumns);
  V = matrix(1,Ncolumns,1,Ncolumns);
  s = vector(1,Ncolumns);
  
  //fprintf(stderr, "pinv: memory successfully allocated!\n");
  
  /*
    //deep-copy matrix A into matrix U 
    for (i=1;i<=Nrows;i++) {
    //deep-copy row i of matrix A into row i of matrix U
    //pta = &(A[i][1]);
    //ptb = &(U[i][1]);
    for (j=1;j<=Ncolumns;j++)
    U[i][j] = A[i-1][j-1];//(float)(*pta++);
    //CP_VEC(&(A[i][1]), &(U[i][1]), Ncolumns, float, float);
    }
  */
  
  //deep-copy matrix A into matrix U 
  for (i=1;i<=Nrows;i++) {
    //deep-copy row i of matrix A into row i of matrix U
    //pta = &(A[i][1]);
    ptb = &(U[i][1]);
    for (j=0;j<Ncolumns;j++)
      *ptb++ = A[i-1][j];//(float)(*pta++);
    //CP_VEC(&(A[i][1]), &(U[i][1]), Ncolumns, float, float);
  }
 
  //some kind of magic!
  fprintf(stderr, "");
  //fprintf(stderr, "pinv: matrix copy completed!\n");
  
  //calculate the singular value decomposition
  svdcmpFloat(U, Nrows, Ncolumns, s, V);
  
  //fprintf(stderr, "pinv: singular value decomposition completed!\n");
  
  //for (i=1;i<=Ncolumns;i++)
  //   fprintf(stderr, "pinv: s[%2i] = %g\n", i, s[i]);
  //DEBUG("pinvT: s[%2i] = %g\n", i, s[i]);
  
  /* calculate the pseudoinverse matrix                */
  /* remove the near singular values from the vector s */
  
  //find value of the biggest element of s[]
  pta = &(s[1]);
  max = (float)*pta++;
  for (i=1;i<Ncolumns;i++) {
    max = max>(float)*pta ? max : (float)*pta;
    //max = MAX(max, (float)*pta);
    pta++;
  }
  //MAX_ELEMENT(&(s[1]), max, Ncolumns, float, float);
  
  //  fprintf(stderr, "pinv: max s[] element found is %d\n", max);
  
  //A becomes the *transverse* of the pseudoinverse matrix
  for (i=1;i<=Ncolumns;i++)
    for (j=1;j<=Nrows;j++) {
      A[j-1][i-1] = 0;
      for (k=1;k<=Ncolumns;k++)
        A[j-1][i-1] += V[i][k] * U[j][k] * (s[k]/max < SVD_TRUNC ? 0 : 1/s[k]);
    }
  
  //  fprintf(stderr, "pseudoinverse found, about to clean up...\n");
  
  free_matrix(U,1,Nrows,1,Ncolumns);
  free_matrix(V,1,Ncolumns,1,Ncolumns);
  free_vector(s,1,Ncolumns);
  return(0);
}

//Given a matrix a[1..m][1..n], this routine computes its singular value
//decomposition, A = U*W*V'. The matrix U replaces a on output. The diagonal
//matrix of singular values W is output as a vector w[1..n]. The matrix V
//(not the transpose V') is output as v[1..n][1..n].
void svdcmpFloat(float **a, int m, int n, float w[], float **v)
{
  float pythag(float a, float b);
  int flag,i,its,j,jj,k,l,nm;
  float anorm,c,f,g,h,s,scale,x,y,z,*rv1;
  
  rv1=vector(1,n);
  g=scale=anorm=0.0;
  // Householder reduction to bidiagonal form.
  for (i=1;i<=n;i++) {
    l=i+1;
    rv1[i]=scale*g;
    if (i <= m) {
      for (k=i;k<=m;k++) scale += fabs(a[k][i]);
      if (scale) {
	for (k=i;k<=m;k++) {
	  a[k][i] /= scale;
	  s += a[k][i]*a[k][i];
	}
	f=a[i][i];
	g = -SIGN(sqrt(s),f);
	h=f*g-s;
	a[i][i]=f-g;
	for (j=l;j<=n;j++) {
	  for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
	  f=s/h;
	  for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
	}
	for (k=i;k<=m;k++) a[k][i] *= scale;
      }
    }
    w[i]=scale *g;
    g=s=scale=0.0;
    if (i <= m && i != n) {
      for (k=l;k<=n;k++) scale += fabs(a[i][k]);
      if (scale) {
	for (k=l;k<=n;k++) {
	  a[i][k] /= scale;
	  s += a[i][k]*a[i][k];
	}
	f=a[i][l];
	g = -SIGN(sqrt(s),f);
	h=f*g-s;
	a[i][l]=f-g;
	for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
	for (j=l;j<=m;j++) {
	  for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
	  for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
	}
	for (k=l;k<=n;k++) a[i][k] *= scale;
      }
    }
    anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
  }
  // Accumulation of right-hand transformations.
  for (i=n;i>=1;i--) {
    if (i < n) {
      if (g) {
        //Double division to avoid possible underflow
	for (j=l;j<=n;j++) v[j][i]=(a[i][j]/a[i][l])/g;
	for (j=l;j<=n;j++) {
	  for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
	  for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
	}
      }
      for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
    }
    v[i][i]=1.0;
    g=rv1[i];
    l=i;
  }
  // Accumulation of left-hand transformations.
  for (i=IMIN(m,n);i>=1;i--) {
    l=i+1;
    g=w[i];
    for (j=l;j<=n;j++) a[i][j]=0.0;
    if (g) {
      g=1.0/g;
      for (j=l;j<=n;j++) {
	for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
	f=(s/a[i][i])*g;
	for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
      }
      for (j=i;j<=m;j++) a[j][i] *= g;
    } else for (j=i;j<=m;j++) a[j][i]=0.0;
    ++a[i][i];
  }
  // Diagonalization of the bidiagonal form:
  // Loop over singular values, and over allowed iterations.
  for (k=n;k>=1;k--) {
    for (its=1;its<=30;its++) {
      flag=1;
      // Test for splitting. Note that rv1[1] is always zero.
      for (l=k;l>=1;l--) {
	nm=l-1;
	if ((float)(fabs(rv1[l])+anorm) == anorm) {
	  flag=0;
	  break;
	}
	if ((float)(fabs(w[nm])+anorm) == anorm) break;
      }
      if (flag) {
	c=0.0;  // Cancellation of rv1[l], if l > 1.
	s=1.0;
	for (i=l;i<=k;i++) {
	  f=s*rv1[i];
	  rv1[i]=c*rv1[i];
	  if ((float)(fabs(f)+anorm) == anorm) break;
	  g=w[i];
	  h=pythag(f,g);
	  w[i]=h;
	  h=1.0/h;
	  c=g*h;
	  s = -f*h;
	  for (j=1;j<=m;j++) {
	    y=a[j][nm];
	    z=a[j][i];
	    a[j][nm]=y*c+z*s;
	    a[j][i]=z*c-y*s;
	  }
	}
      }
      z=w[k];
      // Convergence.
      if (l == k) {
	// Singular value is made nonnegative.
	if (z < 0.0) {
	  w[k] = -z;
	  for (j=1;j<=n;j++) v[j][k] = -v[j][k];
	}
	break;
      }
      if (its == 30)
      { 
      	// this error routine bails out, calling exit(1) ... NO GOOD!!
      	// in a realtime app, stuff just can be bailing out on ya??
        nrerror("no convergence in 30 svdcmp iterations");
      }
      // Shift from bottom 2-by-2 minor.
      x=w[l];
      nm=k-1;
      y=w[nm];
      g=rv1[nm];
      h=rv1[k];
      f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
      g=pythag(f,1.0);
      f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
      c=s=1.0;
      // Next QR transformation.
      for (j=l;j<=nm;j++) {
	i=j+1;
	g=rv1[i];
	y=w[i];
	h=s*g;
	g=c*g;
	z=pythag(f,h);
	rv1[j]=z;
	c=f/z;
	s=h/z;
	f=x*c+g*s;
	g = g*c-x*s;
	h=y*s;
	y *= c;
	for (jj=1;jj<=n;jj++) {
	  x=v[jj][j];
	  z=v[jj][i];
	  v[jj][j]=x*c+z*s;
	  v[jj][i]=z*c-x*s;
	}
	z=pythag(f,h);
	w[j]=z;
	// Rotation can be arbitrary if z = 0.
	if (z) {
	  z=1.0/z;
	  c=f*z;
	  s=h*z;
	}
	f=c*g+s*y;
	x=c*y-s*g;
	for (jj=1;jj<=m;jj++) {
	  y=a[jj][j];
	  z=a[jj][i];
	  a[jj][j]=y*c+z*s;
	  a[jj][i]=z*c-y*s;
	}
      }
      rv1[l]=0.0;
      rv1[k]=f;
      w[k]=x;
    }
  }
  free_vector(rv1,1,n);
}
