/*
  Computes Distances between two Datasets.
  MATLAB Mex File
  
  27 March 1997 ATC Created
  
  X : (Mx x N) 1. Dataset Matrix
  Columns correspond to atributes and rows correspond to individual samples.
  Y : (My x N) 2. Dataset Matrix
  
  if the third parameter is a scalar :
  p : Minkowski Distance ( Default = 2 : Euclidian Distance )
  else if it is a matrix
  A : Linear transformation of Mahalanobis Norm
  
  C : (Mx x My) Pairwise distances of elements in X and Y.
  C(i,j) = d(X(i), Y(j))
  
  Examples :
  
  C = distance(X) % == distance(X,X, 2)
  distance(X,Y)   % Euclidian Distance
  distance(X,Y,1) % Manhattan distance
  distance(X,Y,Inf) % Maximum distance
  distance(X,Y,A)  % Mahalanobis distance

*/
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "mex.h"

char *usage_str="Usage : C = distance(X, Y, p);\n";

void mexFunction( int nlhs, Matrix *plhs[], int nrhs, Matrix *prhs[] )
{
  int Mx, My, Mp, Np, N, p = 2, temp, i;
  double *X, *Y, *C, *dp1, *dp2, *dp3, *dp4, P, temp2, *dpA = NULL, *pDelta, *pDelta2;
  Matrix *A, *inv_A, *Delta = NULL, *Delta2=NULL;
  
  if ( nrhs<1 ) mexErrMsgTxt(usage_str);
    if ( nrhs<2 )
      {
	X = mxGetPr(prhs[0]);
	Y = mxGetPr(prhs[0]);
	My = Mx = mxGetM(prhs[0]);
	N = mxGetN(prhs[0]);
      }
    else
      {
	X = mxGetPr(prhs[0]);
	Y = mxGetPr(prhs[1]);
	Mx = mxGetM(prhs[0]);
	temp = mxGetN(prhs[0]);
	My = mxGetM(prhs[1]);
	if ((N = mxGetN(prhs[1])) != temp )
	  {
	    mexErrMsgTxt("X and Y must have the same number of coloumns!\n");
	  };
      }
    if ( nrhs > 2 )
      {
	Mp = mxGetM(prhs[2]);
	Np = mxGetN(prhs[2]);
	if ((Np==1)&&(Mp ==1))
	  {
	    /* The Second Parameter specifies the Minkowski Metric */
	    P = mxGetScalar(prhs[2]);
	    if (P<1)  mexErrMsgTxt("p must be > 0\n");

	    /* Check for Infinity Metric */
	    p = (finite(P) ? (int)P : 0);
	    dpA = NULL;
	  }
      else /* Mahalanobis Metric */
        {
          N = mxGetN(prhs[0]);
          if (Mp!=N || Np!=N) mexErrMsgTxt("A must be square\n");
          A = prhs[2];
          mexCallMATLAB(1, &inv_A, 1, &A, "inv");
          dpA = mxGetPr(inv_A);
          Delta = mxCreateFull(N, 1, REAL);
          pDelta = mxGetPr(Delta);
          Delta2 =  mxCreateFull(N, 1, REAL);
          pDelta2 = mxGetPr(Delta2);
          p = 2;
        }
      }
    else p=2;
    
  /* mexPrintf("p = %d\n",p); */

    plhs[0] = mxCreateFull(Mx, My, REAL);
    C = mxGetPr(plhs[0]);
    
    switch(p)
      {
      case 0: /* L_Infinity Metric */
	if (X==Y)
	  for (dp1 = X; dp1<X+Mx; dp1++)
	    for (dp2 = dp1+1; dp2<Y+My; dp2++)
	      {
		C[dp1-X + (dp2-Y)*Mx] = 0.0;
		for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		  if (C[dp1-X + (dp2-Y)*Mx] < (temp2 = fabs(*dp3 - *dp4)))
		    C[dp1-X + (dp2-Y)*Mx] = temp2;
		
		C[dp2-Y + (dp1-X)*My] = C[dp1-X + (dp2-Y)*Mx];
	      }
	else
	  {
	    for (dp1 = X; dp1<X+Mx; dp1++)
	      for (dp2 = Y; dp2<Y+My; dp2++)
		{
		  C[dp1-X + (dp2-Y)*Mx] = 0.0;
		  for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		    if (C[dp1-X + (dp2-Y)*Mx] < (temp2 = fabs(*dp3 - *dp4)))
		      C[dp1-X + (dp2-Y)*Mx] = temp2;
		}
	    
	  };
	
       break;
       
      case 1:  /* L1 Metric */
	if (X==Y)
	  for (dp1 = X; dp1<X+Mx; dp1++)
	    for (dp2 = dp1+1; dp2<Y+My; dp2++)
	      {
		C[dp1-X + (dp2-Y)*Mx] = 0.0;
		for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		  C[dp1-X + (dp2-Y)*Mx] += fabs(*dp3 - *dp4);
		C[dp2-Y + (dp1-X)*My] = C[dp1-X + (dp2-Y)*Mx];
	      }
	else
	  {
	    for (dp1 = X; dp1<X+Mx; dp1++)
	      for (dp2 = Y; dp2<Y+My; dp2++)
		{
		  C[dp1-X + (dp2-Y)*Mx] = 0.0;
		  for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		    C[dp1-X + (dp2-Y)*Mx] += fabs(*dp3 - *dp4);
		}
	    
	  };
        break;
	
      case 2:  /* L2 Metric */
	if (dpA == NULL)
	  {
       if (X==Y)
	 for (dp1 = X; dp1<X+Mx; dp1++)
	   for (dp2 = dp1+1; dp2<Y+My; dp2++)
	     {
	       C[dp1-X + (dp2-Y)*Mx] = 0.0;
	       for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		 C[dp1-X + (dp2-Y)*Mx] += (*dp3 - *dp4)*(*dp3 - *dp4);
	       C[dp1-X + (dp2-Y)*Mx] = sqrt(C[dp1-X + (dp2-Y)*Mx]);
	       C[dp2-Y + (dp1-X)*My] = C[dp1-X + (dp2-Y)*Mx];
	     }
       else
	 {
	   for (dp1 = X; dp1<X+Mx; dp1++)
	     for (dp2 = Y; dp2<Y+My; dp2++)
	       {
		 C[dp1-X + (dp2-Y)*Mx] = 0.0;
		 for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		   C[dp1-X + (dp2-Y)*Mx] += (*dp3 - *dp4)*(*dp3 - *dp4);
		 C[dp1-X + (dp2-Y)*Mx] = sqrt(C[dp1-X + (dp2-Y)*Mx]);
	       }
	   
	 };
	  } /* end of A = eye(N);*/
	
	else
	  {
	    for (dp1 = X; dp1<X+Mx; dp1++)
	      for (dp2 = Y; dp2<Y+My; dp2++)
		{
		  C[dp1-X + (dp2-Y)*Mx] = 0.0;
		  
		  i = 0;
		  for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		    pDelta[i++] = (*dp3 - *dp4);
		  
		  for (i = 0; i<N; i++)
		    {
		      pDelta2[i] = 0.0;
		      for (dp3 = pDelta, dp4 = dpA + N*i ; dp3<pDelta+N; dp3++, dp4++)
			pDelta2[i] += *dp3 * *dp4;
		    }
		  
		  for (i = 0; i<N; i++)
		    C[dp1-X + (dp2-Y)*Mx] += pDelta[i]*pDelta2[i];
		  
		  C[dp1-X + (dp2-Y)*Mx] = sqrt(fabs(C[dp1-X + (dp2-Y)*Mx]));
		}

	  }; /* End of Mahalanobis Distance */
	
	break;
      default:
	if (X==Y)
	  for (dp1 = X; dp1<X+Mx; dp1++)
	    for (dp2 = dp1+1; dp2<Y+My; dp2++)
	      {
		C[dp1-X + (dp2-Y)*Mx] = 0.0;
		for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		  C[dp1-X + (dp2-Y)*Mx] += pow(fabs(*dp3 - *dp4), (double)p);
		C[dp1-X + (dp2-Y)*Mx] = pow(C[dp1-X + (dp2-Y)*Mx], (double)1.0/p);
		C[dp2-Y + (dp1-X)*My] = C[dp1-X + (dp2-Y)*Mx];
       }
	else
	  {
	    for (dp1 = X; dp1<X+Mx; dp1++)
	      for (dp2 = Y; dp2<Y+My; dp2++)
		{
		  C[dp1-X + (dp2-Y)*Mx] = 0.0;
		  for (dp3 = dp1, dp4 = dp2; dp3 <dp1+N*Mx; dp3+=Mx, dp4+=My)
		    C[dp1-X + (dp2-Y)*Mx] +=  pow(fabs(*dp3 - *dp4), (double)p);
		  C[dp1-X + (dp2-Y)*Mx] = pow(C[dp1-X + (dp2-Y)*Mx], (double)1.0/p);
		}
	    
	  };
	
	break;
      };
    
    if (Delta) mxFreeMatrix(Delta);
    if (Delta2) mxFreeMatrix(Delta2);
    
  };
