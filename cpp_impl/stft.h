
/* forward short time fourier transform */
void stft(double* x, 
          long    inputSizeInSamples,
          int     fftSize, 
          int     hopSize, 
          double* window, 
          double* fi, 
          double* fr);

/* inverse short time fourier transform */
void istft (double* x, 
            long    numberOfTimeSlices,
            int     halfComplexSize, 
            int     hopSize,  
            double* window, 
            double* fi, 
            double* fr);


