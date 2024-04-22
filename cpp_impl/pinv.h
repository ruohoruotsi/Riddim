/*******************************************************/
/*  																									 */	
/* header for psuedoinverse code, included are helper  */
/* matrix and svd methods, that are intricately linked */
/* to this particular implementation.                  */
/*  																									 */	
/*******************************************************/

#ifndef __PINV_H__
#define __PINV_H__


static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static float maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
(maxarg1) : (maxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
(iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

float pythag(float a, float b);
void nrerror(char error_text[]);
float *vector(long nl, long nh);
float **matrix(long nrl, long nrh, long ncl, long nch);
void free_vector(float *v, long nl, long nh);
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch);


#define SVD_TRUNC  1e-6 /* trunctuate singular values smaller than this	(relative to largest)	*/


/*****************************************************************************
 *
 * pseudoinverse of a square or nonsquare matrix
 *   Multiplication of pinv(A) with A gives the identity matrix.
 *   This function replaces the matrix A with the pseudoinverse matrix.
 *
 ****************************************************************************/

int pinv(float **A, 	/* matrix[1..Nrows][1..Ncolumns] */
         int Nrows, 	/* number of rows    */
         int Ncolumns	/* number of columns */
         );

void svdcmpFloat(float **a, 
                 int m, int n, 
                 float w[], 
                 float **v);

#endif /* __PINV_H__ */
