/*  
    iirFilter.h
    
    Computational function that takes two vectors of coefficients
    B & A and filters them with an IIR filter.
    
    iroro orife 14-12-2000
*/

double* iirFilter(double *B, double *A, int filterLength, double *signal, int signalLength);
