/************************************************************************** Initial migration of Off-line version of Riddim to a kosher stand-alone Non-MATLAB based implementation V 0.1 will be an exact replica of Riddim from dartmouth V 0.2 will do the L2 norming and smarter ISA from the MPEG 7 spec,       maybe some realtime, or filterbank impl.  copyright 2001, iroro orife, All Rights Reserved **************************************************************************/#include <iostream>#include <math.h>#include <string.h>// MAC console#include <console.h>#include <SIOUX.h>#include "sndfile.h"   // sndfile I/O#include "stft.h"      // RFFTW based STFT#include "dspUtils.h"  // iroro DSP & Matrix utilities#include "JnS.h"       // J.F. Cardoso Jade code#include "pinv.h"      // psuedoinverse code#include "tnt/tnt.h"   // Template Numerical Toolkit#include "tnt/vec.h"#include "tnt/cmat.h"using namespace TNT; // necessary for TNT#define PI 3.14159265358979323846264338327950288419int main(int argc, char* argv[]){				char *inputFilename;		int sr,i,j; 			SNDFILE *infile;		SF_INFO sfinfo;						/*********************************************************************/		//  Unpack arguments and read in the sound file into memory		/*********************************************************************/		   	// essential for MAC OS		argc = ccommand(&argv);				// make it seem like this is a real application ...		cout << "***********************************" << endl;	  cout << "      Riddim by iroro orife        " << endl;	  cout << "***********************************" << endl << endl << endl;		cout << "    Vital Statistics:     " << endl << endl;				// parse arguments		if(argc != 3)		{				cout << "Usage: Riddim <audio file name> <sample rate> " << endl; 				return 101;		}		else		{				inputFilename = argv[1];        // input filename 				sr            = atoi(argv[2]);  // the sample rate of the file								cout << "input filename   : = " << inputFilename << endl; 				cout << "input samplerate : = " << sr << endl << endl;		}						// load up sound vector		if (! (infile = sf_open_read (inputFilename, &sfinfo)))		{	printf ("Not able to open input file %s.\n", inputFilename) ;			sf_perror (NULL) ;			return  101;		} 		  // print out vital stats, is everything okay? 		cout << "sfinfo.samplerate  == " << sfinfo.samplerate << endl;		cout << "sf.info.samples    == " << sfinfo.samples << endl;		cout << "sfinfo.channels    == " << sfinfo.channels << endl; 		cout << "sfinfo.pcmbitwidth == " << sfinfo.pcmbitwidth << endl;		cout << "sfinfo.format      == " << sfinfo.format << endl;  // SF_FORMAT_AIFF | SF_FORMAT_PCM	  cout << "sfinfo.sections    == " << sfinfo.sections << endl;	  cout << "sfinfo.seekable    == " << sfinfo.seekable << endl;				// init a variable a bit more usable for # of samples 		int inCount = sfinfo.samples; 		  	  // get some memory from the heap to store the whole sound file 	  // need to make sure the MAC app has enough memory allocated to it ... 	  double *inBuf =  new double[inCount]; 	 	  //  read into an array all the goodies here, the last argument "1" is the normalization 	  //  factor, it needs to be one.	  int readcount = sf_read_double (infile, inBuf, inCount, 1)	; 		cout << "the number of double read " << readcount <<  endl; 		 		 		 		/*********************************************************************/		// perform STFT		/*********************************************************************/		 		int fftSize = 16;		int hopSize = 4;		int numComponents = 5;  						// assert(fftSize % 2 == 0);				// create a hanning window of size 128, get from Matlab;    double *hanningWindow = new double[fftSize];    for (i = 0; i < fftSize; i++)    {      hanningWindow[i] = 0.5 - 0.5*cos(2*PI*i/(fftSize-1));		}		// positive frequencies X vector length divided by hop size		// fftSize better be multiple of 2		int M = fftSize/2 + 1;                       		int N = ceil((inCount)/(double)hopSize);		int outputFrequencySize = M*N;				double* fi = new double[M*N];		double* fr = new double[M*N];		cout << "getting ready to do forward transform" << endl;		// make a spectrogram from the input buffer ...	  // return the complex modulus (magnitude) 	  stft(inBuf,	       (long)inCount,	       fftSize,       // fftsize 	       hopSize,       // hop size	       hanningWindow, // window type	       fi,            	       fr);           // frequency vectors returned 	  	  // STFT returns a TALL MATRIX of size N*M, need to transpose it	  // for it to have the correct values for phases and magnitudes	  // later on, since the assumption is an M*N matrix for most	  // of the subsequent calculation	  	  // calculate the complex modulus (magnitude) and 	  // phase of the complex output of stft 	  double* phases = new double[M*N];	  double* magnitudes = new double[M*N];	  	  // make cos(p) + sin(p)*i  cos == real and sin == imag	  // then for each "r" ... need to multiply 	  // r*real phase  and r*imag phase	  double* phaseImag = new double[M*N];	  double* phaseReal = new double[M*N];	  	  for(i = 0; i < outputFrequencySize; i++)	  {	    phases[i] = atan2(fi[i], fr[i]);	    magnitudes[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i]);	    	    // p = cos(p) + i*sin(p) ;  p = phases	    phaseReal[i] = cos(phases[i]);	    phaseImag[i] = sin(phases[i]);	  }	  	  cout << " done with Spectrogram " << endl;	  	 /* TEST CODE THAT INVERTS SPECTROGRAM TO ENSURE THE STFT IS KOSHER	 ///////////////////////////////////////////////////////////////////////////		delete [] inBuf;		inBuf = 0;	  	  inCount = (N+1)*hopSize+fftSize;	  inBuf = new double[inCount];	  memset(inBuf, 0,  inCount*sizeof(double));	  	  // do ISTFT to test kosher-ness of STFT/ISFT logic	  // flop this mofo back 	  istft(inBuf,	  			(long)N, // the number of time slices ... used a loop counter	  			M,       // fftSize/2 + 1, used as size in ifft	  			hopSize,	  			hanningWindow,	  			fi,	  			fr);	  	  cout << " ... writing file" << endl;	  	  SNDFILE *file;	  if (! (file = sf_open_write ("test_out.aiff", &sfinfo))) 		{				printf ("Error : Not able to open output file.\n") ; 			return 1 ; 		} ; 		// normalize 		if (sf_write_double (file, inBuf, (N+1)*hopSize+fftSize, 1) !=  inCount) 			sf_perror (file) ; 		sf_close (file) ; 		///////////////////////////////////////////////////////////////////////////	  */						// do SVD on covariance matrix (maybe include a flag for C*C' or C'*C)	  // dump spectrogram magnitudes into a matrix	  Matrix<double> S(N,M, magnitudes); 	// tall matrix spectrogram = NxM	  S = transpose(S);										// make it a MxN matrix  -- canonical form				  	  // calculate covariance of S = S'*S	  Matrix<double> covS(S*transpose(S)); //  covariance = SxS'	  	  // covariance matrices are always square!!!!	  	  // prepare to do SVD	  double* uu = new double [M*M];	  double* vv = new double [M*M];	  double*  w = new double [M];	  double** u = new double* [M];	  double** v = new double* [M];	   	 	  for (i = 0; i < M; i++)	    u[i] = &(uu[M*i]);	  for (j = 0; j < M; j++)	    v[j] = &(vv[M*j]);	  {	    for (i = 0; i < M; i++)	      for (j = 0; j < M; j++)	        u[i][j] = (covS.getMat1D())[i*covS.num_cols()+j];	  }	 	  // do SVD	  svd(u, M, M, w, v);	   	  // uu and vv are single dimension versions	  // of u,v - the output of the SVD	  Matrix<double> V(M,M, vv);    					// V = MxM 	  	  // the diagonal W from the U*W*V decomposition	  // this logic puts the diagonal W into a matrix	  // to multiply against V	  double* ww = new double[M*M];						  for (i = 0; i < M; i++)	  {	    for (j = 0; j < M; j++)	    {	      // vv[i*N+j] = v[i][j]; // this should be redundant ...	      if(i == j)	        ww[i*M+j] = w[i];	      else	        ww[i*M+j] = 0;	    }	  }	 		// W = MxM, for example: 65x65	  Matrix<double> W(M,M, ww); // ww is w in a usable format (since vec*mat doesn't give a mat)	  // v = w*v': multiply diagonal matrix by V'		V = W*transpose(V);	  	  // take the first numComponent basis vectors   	  // equivalent to the minus variance step of W'	  // V = V.newsize(numComponents, M);	  Matrix<double> newV(numComponents, M, V.getMat1D());	  	  Matrix<double> spectroProjection(newV*S);   // project spectrogram onto top basis vectors	  																				    // 5*M M*N --> spectroProjection = 5 * N 	  	  // clean up shop a little bit ...	  delete [] uu; delete [] w; 	  delete [] vv; delete [] ww; 	  delete [] u;  delete [] v;			// need a temp vector cos, jade jacks up input stuff ... no good!!	 double *t = new double[numComponents*N];	 	 /*	  for(int k = 0; k < numComponents*N; k++)	  {	  	temp[k] = (spectroProjection.getMat1D())[k];	  }	  */	  	  ///////////////////	  // Uptil here was good	  // 	  	  t[0] = 19.6079;   t[1] = 28.3818;  t[2] = 33.4632;  t[3] = 43.6573;   t[4] = 43.1932;    t[5] = 0.0232;    t[6] = 0.0135;   t[7] = 0.0157;   t[8] = -0.0100;   t[9] = -0.0214;    t[10] = -0.0045;  t[11] = -0.0034; t[12] = 0.0095;  t[13] = -0.0058;  t[14] = 0.0027;    t[15] = -0.0001;  t[16] = 0.0000;  t[17] = 0.0000;  t[18] = 0.0001;   t[19] = -0.0001;    t[20] = -0.0000;  t[21] = 0.0000;  t[22] = -0.0000; t[23] = -0.0000;  t[24] = 0.0000;	  	  // call Jade	  double *mixingMatrix = new double[numComponents * numComponents];	  Jade(mixingMatrix, t, 5, 5); 	// Bad!! spectroProj is changed in jade?!	  // call psuedoinverse	  double *tt = psuedoinverse(mixingMatrix, numComponents, numComponents, 1e-12);	  	  // matricize mixing matrix numComponents x numComponents	  Matrix<double> mx(numComponents, numComponents, tt);	  mx = transpose(mx);  // need this because psuedoinverse returns the tranpose of A'	  	  Matrix<double> c = mx * spectroProjection;  // c = w * c :line 123   c == 5 * N	  	  // wf = w * wf  : line 126  wf = 5*5 x 5*M == 5*M	  Matrix<double> cww(mx * newV);   	  cww = transpose(cww);    // cww = Mx5	  	  ////////////////////////////////////////////	  // call psuedoinverse, cw = pinv(wf); line 131  aa = M*5	  double *aa = psuedoinverse(cww.getMat1D(), M, numComponents,1e-12);	  Matrix<double> cw(numComponents, M, aa);	  // cw = transpose(cw);      // necessary since psuedoinverse returns A' transpose	  	  // Reconstruct spectrogram sub-spaces ...	 	  // clean up input buffer for use with output matrices	 	delete [] inBuf;		inBuf = 0;	 	  //	  for (int m = 0; m < numComponents; m++)	  {	  	// Matrix c =  5 * N	  	// Matrix cw = M * 5	    // so c(1,:) = N cols, cw(:,1) = M rows so, cw * c = M*N spectrogram	    //	    // M*1 x 1*N	  		  	double *firstRowCW = new double[M];	  	// get first row of cw	  	for(i = 0; i < M; i++)	  	{	  		firstRowCW[i] = (cww.getMat1D())[m*numComponents + i];	  	}	  	Matrix<double> oCW(M,1, firstRowCW);	  		  		  	double *firstColumnC = new double[N];	  	// get first column of c	    for(i = 0; i < N; i++)	    {	    	firstColumnC[i] = (c.getMat1D())[m*numComponents + i];	    } 	    Matrix<double> oC(1, N, firstColumnC);	  	    	    // do outerproduct 	    Matrix<double> outerproduct = oCW * oC; // should give an MxN matrix	 	 		// get magnitudes from outerproduct matrix	 		double* mag = outerproduct.getMat1D();	 			 		// a + bi = r(cos(p) + i*sin(p))   r = mag, cosp = phaseReal sinp = phaseImag	 		// p = phase	 		// construct final imag, real matrices for the ISTFT	 		for(i = 0; i < M*N; i++)	 		{	 			phaseReal[i] = mag[i] * phaseReal[i];	    	phaseImag[i] = mag[i] * phaseImag[i];	 		}	  			 // TEST CODE THAT INVERTS SPECTROGRAM TO ENSURE THE STFT IS KOSHER			 ///////////////////////////////////////////////////////////////////////////			  			  inCount = (N+1)*hopSize+fftSize;			  inBuf = new double[inCount];			  memset(inBuf, 0,  inCount*sizeof(double));			  			  // do ISTFT to test kosher-ness of STFT/ISFT logic			  // flop this mofo back 			  istft(inBuf,			  			(long)N, // the number of time slices ... used a loop counter			  			M,       // fftSize/2 + 1, used as size in ifft			  			hopSize,			  			hanningWindow,			  			phaseImag,			  			phaseReal);			  			  cout << " ... writing file #: " << m << endl;			  			  SNDFILE *file;				  				switch(m)				{				  			  	case 0:	if (! (file = sf_open_write ("test_out_0.aiff", &sfinfo))) 									{		printf ("Error : Not able to open output file 0.\n") ; return 1 ; 									} ; break;										case 1: if (! (file = sf_open_write ("test_out_1.aiff", &sfinfo))) 									{		printf ("Error : Not able to open output file 1.\n") ; return 1 ; 									} ; break; 					case 2: if (! (file = sf_open_write ("test_out_2.aiff", &sfinfo))) 									{		printf ("Error : Not able to open output file 2.\n") ; return 1 ; 									} ; break;											case 3: if (! (file = sf_open_write ("test_out_3.aiff", &sfinfo))) 									{		printf ("Error : Not able to open output file 3.\n") ; return 1 ; 									} ; break;										case 4: if (! (file = sf_open_write ("test_out_4.aiff", &sfinfo))) 									{		printf ("Error : Not able to open output file 4.\n") ; return 1 ; 									} ; break;   										default: break;				}				// normalize 				if (sf_write_double (file, inBuf, (N+1)*hopSize+fftSize, 1) !=  inCount) 					sf_perror (file) ; 				sf_close (file) ; 				///////////////////////////////////////////////////////////////////////////			  // */	    	    // % Scale sound and write it out			// s = s - mean( s);			// m = 1.1*max( abs( s(:)));			// wavwrite( s/m, sr, '/usr/tmp/foo.wav')   	    delete [] firstRowCW;			delete [] firstColumnC;     	  }	  	// clean up memory stuff	delete [] inBuf;    delete [] fi;	delete [] fr;  delete [] hanningWindow;			delete [] phases;  delete [] magnitudes;     delete [] mixingMatrix;  	delete [] phaseReal; 	delete [] phaseImag; 	 	// bye	return 0;}