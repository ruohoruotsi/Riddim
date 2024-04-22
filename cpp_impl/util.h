/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this code and associated documentation,  provided       *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *     Copyright (c) held by Dianne Cook                    *
 *  All Rights Reserved.                                    *
 *                                                          *
 *  Questions and comments are welcome, and I request       *
 *  that you share any modifications with me.               *
 *                                                          *
 *                Dianne Cook                               *
 *             dicook@iastate.edu                           *
 *                                                          *
 ************************************************************/

#include <math.h>

#ifdef _UTIL_C

float max_vec(float *x, int n);
float min_vec(float *x, int n);
float mean_fn(float *x1, int n, int *rows_in_plot);
float mean_fn2(float *x1, float *x2, int n, int *rows_in_plot);
float mean_fn3(float *x1, float *x2, float *wgts, int n, int *rows_in_plot);
float mean_fn5(float *x1, float *x2, float *x3, float *x4, float *wgts, int n, int *rows_in_plot);
float calc_norm(float *x, int n);
void  norm(float *x, int n);
float inner_prod(float *x1, float *x2, int n);
void  gram_schmidt(float *x1, float *x2, int n);

float sq_inner_prod(float *x1, float *x2, int n);
void mat_mult(float **a, float **b, float **matprod, int m, int *mv, int n, int p);
void tmat_mult(float **a, float **b, float **matprod, int m, int n, int p, int *pv);

#else

extern float max_vec(float *x, int n);
extern float min_vec(float *x, int n);
extern float mean_fn(float *x1, int n, int *rows_in_plot);
extern float mean_fn2(float *x1, float *x2, int n, int *rows_in_plot);
extern float mean_fn3(float *x1, float *x2, float *wgts, int n, int *rows_in_plot);
extern float mean_fn5(float *x1, float *x2, float *x3, float *x4, float *wgts, int n, int *rows_in_plot);
extern float calc_norm(float *x, int n);
extern void  norm(float *x, int n);
extern float inner_prod(float *x1, float *x2, int n);
extern void  gram_schmidt(float *x1, float *x2, int n);
extern float sq_inner_prod(float *x1, float *x2, int n);
extern void mat_mult(float **a, float **b, float **matprod, int m, int *mv, int n, int p);
extern void tmat_mult(float **a, float **b, float **matprod, int m, int n, int p, int *pv);

#endif
