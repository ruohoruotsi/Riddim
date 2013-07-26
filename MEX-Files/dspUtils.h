/**********************************************************************/
/*                                                                    */   
/*  Various dsp utility methods                                       */
/*  iirFilter - 5 tap IIR Filte                                       */
/*  convolution -                                                     */
/*  maximum - returns the maximum of x[i] and y for all i [0,xlen]    */   
/*  raisedCosine -                                                    */   
/*  gaussian -                                                        */   
/*                                                                    */   
/**********************************************************************/

#define PI 3.14159265358979323846264338327950288419

double* iirFilter(double *B, 
		  double *A, 
		  int filterLength, 
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

double inner_product(double *x, 
		     double *y, 
		     int len);

int returnMinimum(int x, 
		  int y);

int createMAT(const char *file, 
	      double *data, 
	      int datalen);

double* seventhOrderIIRFilter(double *B, 
			      double *A, 
			      double *signal, 
			      int signalLength);
