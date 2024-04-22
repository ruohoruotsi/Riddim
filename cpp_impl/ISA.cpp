/**************************************************************************
 **************************************************************************
 *
 * iroro's implementation of the canonical ISA (based on gkc) by
 * Michael A. Casey.
 *
 * copyright 2001, iroro orife, All Rights Reserved
 *
 **************************************************************************
 **************************************************************************/

#include <iostream>
#include <fstream> // for file writing utilities
#include <math.h>
#include <string.h>

#include "sndfile.h"   // sndfile I/O
#include "stft.h"      // RFFTW based STFT
#include "dspUtils.h"  // iroro DSP & Matrix utilities

extern "C"{
#include "JnS.h"       // J.F. Cardoso Jade code
}

// #include "pinv.h"      // psuedoinverse code
#include "ISA.h"       // Independent Subspace Analysis (ISA)

#include "tnt/tnt.h"   // Template Numerical Toolkit (TNT)
#include "tnt/vec.h"
#include "tnt/cmat.h"

using namespace std;
using namespace TNT; // necessary for TNT

// implemented as a utility methods below, not part of public
// interface yet.
Matrix<double> matrixMax(Matrix<double> x, int M, int N);
Matrix<double> Whiten(Matrix<double> X);

int ISA(double* inBuf,
        long inCount,   
        int sampleRate,
        int fftSize,
        int hopSize,
        int numComponents)
{
  int i,j;
  
  // ensure arguments is kosher
  assert(fftSize % 2 == 0);
  
  /********************************************************************/
  /* perform STFT                                                     */
  /********************************************************************/

  // create a hanning window of size 128, get from Matlab;
  double *hanningWindow = new double[fftSize];
  for (i = 0; i < fftSize; i++)
  {
    hanningWindow[i] = 0.5 - 0.5*cos(2*PI*i/(fftSize-1));
  }
  
  // positive frequencies X vector length divided by hop size
  // fftSize better be multiple of 2
  int M = fftSize/2 + 1;                       
  int N = ceil((inCount)/(double)hopSize);
  int outputFrequencySize = M*N;
  
  double* fi = new double[M*N];
  double* fr = new double[M*N];
  
  cout << "projecting time-series onto new basis (spectrogram) ..." 
       << endl << endl;
  
  // make a spectrogram from the input buffer ...
  // return the complex modulus (magnitude) 
  stft(inBuf,
       inCount,
       fftSize,       // fftsize 
       hopSize,       // hop size
       hanningWindow, // window type
       fi,            
       fr);           // frequency vectors returned 
  
  // STFT returns a TALL MATRIX of size N*M, need to transpose it
  // for it to have the correct values for phases and magnitudes
  // later on, since the assumption is an M*N matrix for most
  // of the subsequent calculation
  
  cout << " calculating phase and magnitudes values of spectrogram "
       << endl;
  
  // calculate the complex modulus (magnitude) and 
  // phase of the complex output of stft 
  double* phases = new double[M*N];
  double* magnitudes = new double[M*N];
  
  // make cos(p) + sin(p)*i  cos == real and sin == imag
  // then for each "r" ... need to multiply 
  // r*real phase  and r*imag phase
  double* phaseImag = new double[M*N];
  double* phaseReal = new double[M*N];
  
  for(i = 0; i < outputFrequencySize; i++)
  {
    phases[i] = atan2(fi[i], fr[i]);
    magnitudes[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i]);
    
    // p = cos(p) + i*sin(p) ;  p = phases
    phaseReal[i] = cos(phases[i]);
    phaseImag[i] = sin(phases[i]);
  }
  
  // do SVD on covariance matrix (maybe include a flag for C*C' or C'*C)
  // dump spectrogram magnitudes into a matrix
  Matrix<double> S(N,M, magnitudes); 	// tall matrix spectrogram = NxM
  S = transpose(S); // make it a MxN matrix  -- canonical form			
  
  // io hacky: replace that instead of the puppy above
  //  Matrix<double> S; cin >> S;
  
  cout << "calculating covariance matrix of spectrogram ..." << endl;

  // calculate covariance of S = S'*S
  Matrix<double> covS(S*transpose(S)); //  covariance = SxS'
  
  // covariance matrices are always square!!!!
  // prepare to do SVD
  double* uu = new double [M*M];
  double* vv = new double [M*M];
  double*  w = new double [M];
  double** u = new double* [M];
  double** v = new double* [M];
  
  for (i = 0; i < M; i++)
    u[i] = &(uu[M*i]);
  for (j = 0; j < M; j++)
    v[j] = &(vv[M*j]);
  {
    for (i = 0; i < M; i++)
      for (j = 0; j < M; j++)
        u[i][j] = (covS.getMat1D())[i*covS.num_cols()+j];
  }
  
  cout << "doing SVD of covariance matrix ... " << endl << endl;

  // do SVD
  svd(u, M, M, w, v);
  
  // IO HACK DECEMBER 10th 2001
  // for some reason V returned from SVD == 1-V returned
  // by MATLAB SVD in GKC, so we correct here 
  //  for(i = 0; i < M*M; i++)
  //  vv[i] = 1 - vv[i];  // go figure
  
  // uu and vv are single dimension versions of u,v - the output of the SVD
  Matrix<double> V(M,M, vv); // V = MxM 
  
  // the diagonal W from the U*W*V decomposition
  // matricize W
  double* ww = new double[M*M];					
  for (i = 0; i < M; i++)
  {
    for (j = 0; j < M; j++)
    {
      // vv[i*N+j] = v[i][j]; // this should be redundant ...
      if(i == j)
        ww[i*M+j] = w[i];
      else
        ww[i*M+j] = 0;
    }
  }	 
  
  // W = MxM, for example: 65x65
  Matrix<double> W(M,M, ww); // ww is w in a usable format (since vec*mat doesn't give a mat)
  
  // cout << " this is SVD output W" << endl;
  // cout << W;

  
  // v = w*v': multiply diagonal matrix by V'
  V = W*transpose(V);

  // cout <<" this is W*V'" << endl;
  // cout << V;
  
  // SO FAR THE DATA MORE OR LESS MATCHES, @ LEAST THE STRUCTURE IS
  // the same altho the scale varies ... YOUR STUFF == (1 - GKC STUFF)

  // take the first numComponent basis vectors   
  // equivalent to the minus variance step of W'
  // V = V.newsize(numComponents, M);
  Matrix<double> newV(numComponents, M, V.getMat1D());
  
  // IO HACK DECEMBER 10th 2001
  // for some reason V returned from SVD == 1-V returned
  // by MATLAB SVD in GKC, so we correct h
  for(i = 0; i < numComponents*M; i++)
    newV.getMat1D()[i]  = 1 - newV.getMat1D()[i];
  
  // cout << "new vector from the minux variance step of W'" << endl;
  // cout << newV;

  Matrix<double> spectroProjection(newV*S);   // project spectrogram onto top basis vectors
  // 5*M M*N --> spectroProjection = 5 * N 
  
  // clean up shop a little bit ...
  delete [] uu; delete [] w; 
  delete [] vv; delete [] ww; 
  delete [] u;  delete [] v;

  /**************************************************************/  
  /**************************************************************/  

  // get whitening matrix
  Matrix<double> white = Whiten(spectroProjection);

  // perform whitening procedure (only do this if not rigging mixing mat!)
  spectroProjection = white*spectroProjection;
  
  // need a temp vector cos, jade modifies spectroProjection
  double *temp = new double[numComponents*N];
  for(int k = 0; k < numComponents*N; k++)
    temp[k] = (spectroProjection.getMat1D())[k];
  
  cout << "calling Jade to get back unmixing matrix ... " << endl << endl;
  
  // call Jade, already doing whitening, so comment out that code
  double *mixingMatrix = new double[numComponents * numComponents];
  Jade(mixingMatrix, temp, numComponents, N);  

  // Jade results
  Matrix<double> tempmofo(numComponents, numComponents, mixingMatrix);

  // subsituting this MATLAB matrix for above jade mixing matrix-matricization
  //Matrix<double> tempmofo(2, 2, 
  //                        "6362541.98781313   4452380.86168445"
  //                        "-0119749.39621986   0171124.30071310");
  
  // call psuedoinverse
  double *tt = psuedoinverse(tempmofo.getMat1D(), numComponents, numComponents, 1e-22);
  
  // matricize pinv'd mixing matrix  ==  numComponents x numComponents
  Matrix<double> mx(numComponents, numComponents, tt);
  Matrix<double> c = mx * spectroProjection;  // c = w * c :line 123  // c == 5 * N
  
  // wf = w * wf  : line 126  wf = 5*5 x 5*M == 5*M
  Matrix<double> cww(mx * newV);   

  // transpose because the SVD wants tall matrices w/ N>M
  cww = transpose(cww);    // cww = Mx5  

  // call psuedoinverse, cw = pinv(wf); line 131  aa = M*5
  double *aa = psuedoinverse(cww.getMat1D(), M, numComponents, 1e-12);
  Matrix<double> cw(numComponents, M, aa); // implicit conversion
  cw = transpose(cw);      // necessary We transposed original matrix
  // because it wasn't a tall matrix... this undoes operation

  cout << "reconstructing spectrogram subspaces ... " << endl;
  
  // Reconstruct spectrogram sub-spaces ...
  delete [] inBuf;
  inBuf = 0;

  //////////////////////////////////////////  
  // init matrices used in re-synthesis
  Matrix<double> oC;
  Matrix<double> oCW;
  Matrix<double> outerproduct;
  
  // components of outerproduct
  double *firstRowC     = new double[N];
  double *firstColumnCW = new double[M]; 
  
  double* pr = new double[M*N];
  double* pi = new double[M*N];
  
  for (int m = 0; m < numComponents; m++)
  {
    memset(firstRowC, 0,  N*sizeof(double));    
    for(i = 0; i < N; i++)     // get first row of c
    {
      firstRowC[i] = (c.getMat1D())[m*N + i];
    }
    oC = Matrix<double>(1, N, firstRowC); 
    
    memset(firstColumnCW, 0,  M*sizeof(double));
    for(i = 0; i < M; i++)     // get first column of cw
    {
      firstColumnCW[i] = (cw.getMat1D())[i*numComponents + m];
    } 
    oCW = Matrix<double>(M, 1, firstColumnCW); 

    // do outerproduct 
    outerproduct = Matrix<double>(oCW * oC); // should give an MxN matrix
   
    // transpose outerproduct to get a TALL N*M matrix before calling ISTFT 
    outerproduct = transpose(outerproduct); 

    // get magnitudes from outerproduct matrix
    double* mag = outerproduct.getMat1D();
   
    // a + bi = r(cos(p) + i*sin(p))   r = mag, cosp = phaseReal sinp = phaseImag
    // so r*cos(p) goes in as real, and r*sin(p) as imag.
    // p = phase
    // construct final imag, real matrices for the ISTFT
    for(i = 0; i < M*N; i++)
    {
      pr[i] = mag[i] * phaseReal[i];
      pi[i] = mag[i] * phaseImag[i];
    }
    
    ///////////////////////////////////////////////////////////////////////////

    inCount = (N+1)*hopSize+fftSize;
    inBuf = new double[inCount];
    memset(inBuf, 0,  inCount*sizeof(double));
    
    // flop this mofo back 
    istft(inBuf,
          (long)N, // the number of time slices ... used a loop counter
          M,       // fftSize/2 + 1, used as size in ifft
          hopSize,
          hanningWindow,
          pi,
          pr);
    
    ///////////////////////////////////////////////////////////////////////////
    
    // remove mean
    double mean = getMean(inBuf, inCount);
    for(i = 0; i < inCount; i++)
      inBuf[i] = inBuf[i] - mean;
    
    // scale sound
    normalize(inBuf, inCount);
    
    cout << "... writing file #: " << m << endl;
    
    SNDFILE *file;
    SF_INFO sfinfo;
    
    // set up sfinfo    
    sfinfo.samplerate  = sampleRate;
    sfinfo.channels    = 1;
    sfinfo.samples     = inCount;
    sfinfo.pcmbitwidth = 16;
    sfinfo.format      = SF_FORMAT_WAV | SF_FORMAT_PCM; // (65537)
    sfinfo.sections    = 1;
    sfinfo.seekable    = 1;

    // the physical maximum is 12 components
    switch(m)
    {
      case 0: if (! (file = sf_open_write ("subspace_outfile_0.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 0.\n") ; return 1 ; 
      } ; break;
      
      case 1: if (! (file = sf_open_write ("subspace_outfile_1.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 1.\n") ; return 1 ; 
      } ; break; 
      
      case 2: if (! (file = sf_open_write ("subspace_outfile_2.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 2.\n") ; return 1 ; 
      } ; break;
      
      case 3: if (! (file = sf_open_write ("subspace_outfile_3.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 3.\n") ; return 1 ; 
      } ; break;
      
      case 4: if (! (file = sf_open_write ("subspace_outfile_4.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 4.\n") ; return 1 ; 
      } ; break;   

      case 5: if (! (file = sf_open_write ("subspace_outfile_5.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 5.\n") ; return 1 ; 
      } ; break;   

      case 6: if (! (file = sf_open_write ("subspace_outfile_6.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 6.\n") ; return 1 ; 
      } ; break;   

      case 7: if (! (file = sf_open_write ("subspace_outfile_7.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 7.\n") ; return 1 ; 
      } ; break;   

      case 8: if (! (file = sf_open_write ("subspace_outfile_8.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 8.\n") ; return 1 ; 
      } ; break;   

      case 9: if (! (file = sf_open_write ("subspace_outfile_9.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 9.\n") ; return 1 ; 
      } ; break;   

      case 10: if (! (file = sf_open_write ("subspace_outfile_10.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 10.\n") ; return 1 ; 
      } ; break;   

      case 11: if (! (file = sf_open_write ("subspace_outfile_11.wav", &sfinfo))) 
      {		printf ("Error : Not able to open output file 11.\n") ; return 1 ; 
      } ; break;   

      default: break;
    }

    // normalize 
    if (sf_write_double (file, inBuf, inCount, 1) !=  inCount) 
      sf_perror (file) ; 
    
    sf_close (file) ; 

    ///////////////////////////////////////////////////////////////////////////
  }
  
  // clean up heap 
  delete [] firstRowC;
  delete [] firstColumnCW;     
  delete [] inBuf;
  
  // clean up heap 
  delete [] fi;
  delete [] fr;
  delete [] hanningWindow;	
  
  delete [] phases;
  delete [] magnitudes;
  
  delete [] mixingMatrix;
  
  delete [] phaseReal;
  delete [] phaseImag;
  
  // bye
  return 0;
}
          
          
/*********************************************************
 *********************************************************
 *
 * return value is a row vector containing the maximum 
 * element from each column. 
 *
 *********************************************************
 *********************************************************/
Matrix<double> matrixMax(Matrix<double> X, int M, int N)
{
  // given an MxN matrix X, we want to examine
  // each column and return the max value.
  
  double* max = new double[N];
  for(int i = 0; i < N; i++)    
    max[i] = 0;
  
  // initialize max matrix structure
  Matrix<double> maxMat(1, N, max);
  
  for(i = 0; i < M*N; i++)
  {
    // i%N should always keep the index into maxMax  inbounds
    if(maxMat.getMat1D()[i%N] <  fabs(X.getMat1D()[i]))
    {
      maxMat.getMat1D()[i%N] = X.getMat1D()[i];
    }
  }
  delete [] max;
  return maxMat;
}


// whiteMat = Whiten(X); returns whiteMat
// to whiten matrix X, perform matrix multiply  X = whiteMat*X
Matrix<double> Whiten(Matrix<double> X)
{
  int i,j;
  int M = X.num_rows();
  int N = X.num_cols();

  cout << " this is the input matrix of size " << M << "x" << N << endl;
  
  // X*X'/T
  Matrix<double> cov(X*transpose(X));
  for(i = 0; i < M*M; i++)
    cov.getMat1D()[i] = cov.getMat1D()[i]/(double)N;
  
  // do io whitening by hand ...
  /**************************************************************/    
  // calculating eigenvalues ...
  // cov holds the X*X'/T, need to implement sqrtm(cov)
  // get eig(cov) = [U,D], cov*V = V*D , sqrt(D)*U = 
  //
  // use SVD to get eigenvalues/eigenvectors of Cov

  // prepare to do SVD
  double* uu = new double [M*M];
  double* vv = new double [M*M];
  double*  w = new double [M];
  double** u = new double* [M];
  double** v = new double* [M];
  for (i = 0; i < M; i++)
    u[i] = &(uu[M*i]);
  for (j = 0; j < M; j++)
    v[j] = &(vv[M*j]);
  {
    for (i = 0; i < M; i++)
      for (j = 0; j < M; j++)
        u[i][j] = (cov.getMat1D())[i*cov.num_cols()+j];
  }
  
  // do [U,W,V] = SVD(cov)
  svd(u, M, M, w, v);
  
  // V
  Matrix<double> V(M,M, vv); // V = MxM 

  double* ww = new double[M*M];					
  for (i = 0; i < M; i++)
  {
    for (j = 0; j < M; j++)
    {
      if(i == j)
        ww[i*M+j] = w[i];
      else
        ww[i*M+j] = 0;
    }
  }

  // W, diagonal
  Matrix<double> W(M,M, ww); 

  // calcualte W = sqrt(W); W being the diagonal matrix from the SVD
  for(i = 0; i< M*M; i++)
    W.getMat1D()[i] = sqrt(fabs(W.getMat1D()[i]));
  
  // calculate the inverse whitening matrix = V*W/V
  Matrix<double> invW(V*W*transpose(V));

  // call psuedoinverse get back Whitenening matrix
  double *white = psuedoinverse(invW.getMat1D(), M, M, 1e-12);
  
  Matrix<double> whiteMat(M, M, white);

  // clean up 
  delete [] uu;   delete [] u;
  delete [] vv;   delete [] v;
  delete [] w;    delete [] ww;

  return whiteMat;
}
