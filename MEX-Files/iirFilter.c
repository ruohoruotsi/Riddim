/*  
    various dsp utility methods 
    
    iirFilter -  Computational function that 
    takes two vectors of coefficients B & A and 
    filters them with an 5 tap IIR filter.
    
    convolution - 

    raisedCosine - 

    gaussian - 

    maximum - 

    by 
    iroro orife 14-12-2000
*/


double* iirFilter(double *B, double *A, int filterLength, double *signal, int signalLength)
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
  x = (double*)malloc((filterLength-1)*sizeof(double));
  y = (double*)malloc((filterLength-1)*sizeof(double));
  output = (double*)malloc((signalLength)*sizeof(double));
  
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
      zeros = B[0]*signal[i] + B[1]*x[0] + B[2]*x[1] + B[3]*x[2] + B[4]*x[3];
      poles =                  A[1]*y[0] + A[2]*y[1] + A[3]*y[2] + A[4]*y[3];
      
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
  
  return output;
}

