/**********************************************************************
 *                                                                    *
 * bandOnsets.cc                                                      *
 *                                                                    *
 * Computational function that calculates the onsets                  *
 * that occur in a specific band                                      *
 *                                                                    *
 * iroro orife copyright 2001                                         *
 **********************************************************************/

#include <iostream>
#include <string.h>
#include <math.h>

extern "C" {
#include "mex.h"
#include "dspUtils.h"
}

#include "BandOnsets.h"

BandOnsets::BandOnsets(){};
BandOnsets::~BandOnsets(){};

/* 
   the output arguments are accumulated in STL vectors since we don't
   know how big they will be, while the input vectors are kept as
   regular arrays. A cleaner and more scalable way of doing things.
*/

/**********************************************************************/
/*     bandOnsets, the grunge work  happens here                      */
/**********************************************************************/
void BandOnsets::getBandOnsets(DoubleVector &peaktimesB, 
                               DoubleVector &peakpressesB,     
                               DoubleVector &peakBgPressesB,   
                               DoubleVector &bankPressesB,   
                               int *peakCount,                        
                               double *signal, 
                               double srate, double timefrac, 
                               double loudtimefrac, 
                               int diffPrecision,
                               double Fdec, double *Bdec, double *Adec,
                               int signalLength,
                               double thresholdValue)
{
  int i, j, k;
  int scanCount, surfboardLength, gaussEnvLength;
  
  /* variables allocated on the heap */
  double *bandSigDec;
  double *surfboard;
  double *gaussEnvelope, *temp;
  double *diffSig;
  
  double smallvalue, sum;
  
  int top, bottom, max_index, diffSigLength;
  int bandSigDecLength, maxRelativeDiffIndex;
  
  int decimation_step = (int)ceil((srate/2)/Fdec);  // IO, is the div 2 necessary
  int timefracsamples = (int)ceil((srate*timefrac/decimation_step)); 
  int ldeviat, rdeviat; 
  
  int riseIdsLength;
  double *riseIds;
  double *customGaussEnv;
  
  /*  D(t) = sig(t + difflen) - sig(t) */
  int difflen = (int)ceil((timefrac*srate)/(diffPrecision * decimation_step));
  
  ///////////////////////////////////////////////////////////////////////
  //  createMAT("iroro_initialSignal.mat", signal, signalLength);
  
  /*
    cout << " thresholdValue = " <<  thresholdValue << endl;
    cout << " diff len = " <<  difflen << endl;
    cout << "filterLength  = " << filterLength << endl;
    cout << " signalLength  = " << signalLength << endl;
    cout << " decimation_step  = " << decimation_step << endl;
  */
  
  /* surfboard calculation, right half of a raised cosine */
  surfboardLength = (int)ceil((2*timefrac*srate)/decimation_step);
  surfboard = new double[surfboardLength];
  
  // cout << " surboardLength  = " << surfboardLength << endl;

  /********************************************************************/
  /* make a surfboard or something                                    */
  /* surfboardlen is right w/ decimation factor                       */
  /********************************************************************/
  for(i=0; i < surfboardLength; i++)
  {
    surfboard[i] = .5*( 1 - cos(2* PI *(.5 + i*.5/surfboardLength)));
    surfboard[i] = 2*surfboard[i]/surfboardLength;
  }
  
  /********************************************************************/
  /* gaussian function                                                */
  /* the size shouuld be timefracsamples = srate*timefrac/decimation  */
  /* i.e. the smallest sample interval perceptible                    */
  /* create a vector that goes from 0 to 1 in increments of           */
  /* 1/timefracsamples . . . that pass that to the gaussianMF         */
  /* sigma = .25 and C = .5                                           */
  /* see GAUSSMF from the Matlab fuzzy toolbox for details.           */
  /********************************************************************/
  gaussEnvLength = timefracsamples;
  if(!fmod(gaussEnvLength, 2)) // if its even, make it odd!
  {    
    gaussEnvLength += 1;
  }
  
  gaussEnvelope = new double[gaussEnvLength];
  for(i = 0; i < gaussEnvLength; i++)
  {
    gaussEnvelope[i] = i*(1.0/(double)timefracsamples);
  }

  /***********************************************/
  /* IO, what is this being hardcoded in here ?? */
  /* .25, .5                                     */
  /***********************************************/
  gaussMF(gaussEnvelope, gaussEnvLength, .25, .5);
  // createMAT("iroro_gaussenv.mat", gaussEnvelope, gaussEnvLength);  


  /********************************************************************/
  /*  half-wave rectify                                               */
  /********************************************************************/
  for(i = 0; i < signalLength; i++)
  {
    if(signal[i] < 0)
    {
      signal[i] = 0;
    }
  }
  //  createMAT("io_after_rectify.mat", signal, signalLength);    


  /********************************************************************/
  /*  jusqu'ici the signal looks sexy                                 */
  /*  now hard threshold it and see what happens                      */
  /********************************************************************/
  for(i = 0; i < signalLength; i++)
  {
    // pass in threshold value . . .
    if(signal[i] <= thresholdValue )
    {
      signal[i] = 0;
    }
  }
  //  createMAT("io_after_thresholding.mat", signal, signalLength);    


  /********************************************************************/
  /* pass signal through a "decimation" filter first then decimate    */
  /********************************************************************/
  bandSigDecLength = (int)(signalLength/decimation_step);
  bandSigDec = new double[bandSigDecLength];
  
  // filter before decimate please, IO check rationale . . .Fdec = ??
  // anti-aliasing lowpass filter?
  signal = seventhOrderIIRFilter(Bdec, 
                                 Adec, 
                                 signal,
                                 signalLength);  
  
  //  createMAT("io_after_decFiiltering.mat", signal, signalLength);  
  // decimate . . .
  for(i = 0, j = 0; i < signalLength && j < bandSigDecLength; 
      j++, i+=decimation_step)
  {
    bandSigDec[j] = signal[i];
  }
  
  /********************************************************************
   * remove numeric precision problems, since may contain small 
   * values of bandSigDec = max(1e-16, bandSigDec) max subroutine
   * this is necessary 
   ********************************************************************/
  //  createMAT("iroro_debug_b4convdecimated.mat", bandSigDec, bandSigDecLength); 
  smallvalue = 1.0E-16;
  maximum(bandSigDec, bandSigDecLength, smallvalue);


  /********************************************************************/
  /* convolution with surfboard (the right half of a raised cosine)   */
  /* a smoothing filter                                               */
  /********************************************************************/
  bandSigDec = conv(bandSigDec, bandSigDecLength, surfboard, surfboardLength);
  //  createMAT("iroro_bandSigDec.mat", bandSigDec, bandSigDecLength);  


  // IO comment need to normalize things here cos they're jacked up
  // NORMALIZE
  //  int divisor = getMaximumIndex(bandSigDec, bandSigDecLength);
  //  for(i = 0; i < bandSigDecLength; i++) 
  //{
  //  bandSigDec[i] = bandSigDec[i]/bandSigDec[divisor];
  // }

  /********************************************************************/
  /* Questions is bankPresses initialized??                           */
  /* is it a 2 dimensional array . . .                                */
  /* not for the needs of this method . . .                           */
  /*                                                                  */
  /* for loop to collect pressure values                              */
  /* loudtimefrac is how many times/second we sample the loudness     */
  /********************************************************************/
  for(i = 0; i < signalLength*loudtimefrac/srate; i++)
  {
    bankPressesB.push_back( bandSigDec[i*(int)ceil((srate/loudtimefrac/decimation_step))] );
  }


  /********************************************************************/
  /* while loops to scan up and down for peaks                        */
  /********************************************************************/
  scanCount = 0;
  *peakCount = 0;
  while(scanCount < bandSigDecLength - difflen)
  {
    // scan to the bottom of a hill 
    while((scanCount < (bandSigDecLength - difflen)) && 
          (bandSigDec[scanCount + difflen] - bandSigDec[scanCount] <= 0))
    {
      scanCount++;
    }
    bottom = scanCount;
    
    // scan to the top of a hill
    while((scanCount < (bandSigDecLength - difflen)) && 
          (bandSigDec[scanCount + difflen] - bandSigDec[scanCount] > 0))
    {
      scanCount++;
    }
    top = scanCount;
    
    // check to see if we are really going back down a hill
    // i.e. negative slope on the other side of top.
    if(bandSigDec[scanCount + difflen] - bandSigDec[scanCount] <= 0)
    {
      top = top - 1;
    }
    
    diffSigLength = top - bottom;
    
    /*****************************************************************/
    /*****************************************************************/
    if(bottom <= top) // this should always be the case
    {
      diffSig = new double[diffSigLength];
      
      // differentiate on the [bottom, top] interval
      for(j = 0; j < diffSigLength; j++)
      {
        diffSig[j] = 
          bandSigDec[difflen + bottom + j] - bandSigDec[bottom + j];
      }
      
      // io debugging in full force.
      //      createMAT("iroro_debug_diffSig.mat", diffSig, diffSigLength);

      // find the max point in diffSig . . . subroutine??
      max_index = getMaximumIndex(diffSig, diffSigLength);
      
      // sum diffSig around the maximum slope . . .
      // using the gaussian slope business . .
      // why min and why not constant?? when timefrac is large
      // and maxid is small 3 then we use maxid-1, no 
      // assumptions here
      ldeviat = returnMinimum((int)(timefracsamples/2), 
                              max_index - 1);
      rdeviat = returnMinimum((int)(timefracsamples/2), 
                              diffSigLength - max_index); 
      
      // make riseids interval 
      // if you did your math right, the slopegauss should
      // be exactly timefracsamples long!! inner product with
      //
      // max_index + riseids will get the max_index centered 
      // on an interval the size of timefracsamples . . do inner 
      // product with gaussian envelope
      
      // do gaussian summing??  this is intimately tied to the 
      // size of the gaussian window!!!
      // fill up peakpressLin . . .
      
      // riseids is the examination interval. maxid + riseids puts
      // maxid right in the middle with ldeviat & rdeviat # on either
      // side.
      
      // create a new vector riseIds where the indicies are on
      // the interval [-ldeviat, rdeviat] with max_index in the
      // middle

      // should remain as is
      // cos of the zero case
      riseIdsLength = ldeviat + rdeviat + 1; 
      riseIds = new double[riseIdsLength];
      
      if (riseIdsLength > 0 
          && (rdeviat + max_index + 1) > diffSigLength)
      {
        riseIdsLength = ldeviat + rdeviat;
      }


      customGaussEnv = new double[riseIdsLength];

      k = (int)(ceil(timefracsamples/2)) + 1;	  
      
      // initialize riseIds
      for(i = 0, j = -ldeviat; i < riseIdsLength; i++, j++)
      {
        riseIds[i] = diffSig[j + max_index];
        customGaussEnv[i] = gaussEnvelope[k + j];
      }

      // what does the envelope look like??
      //      createMAT("iroro_gaussenv.mat", customGaussEnv, riseIdsLength);

      // what does the envelope look like??
      //      createMAT("iroro_riseIds.mat", riseIds, riseIdsLength);


      // add data to peakpressB
      double peakpressLin = inner_product(customGaussEnv, riseIds,
                                          riseIdsLength);
      
      peakpressesB.push_back(peakpressLin);

      // add data to peakBgPressesB
      double peakBgLin = bandSigDec[bottom];
      peakBgPressesB.push_back(peakBgLin);

      
      // calculate maximum value of diffsig/banksig on interval
      // that's a tricky one . . difflen/2 + [bottom, top]
      /****************************************************************/
      /* get the maxrelid . . this looks like a subroutine            */
      /****************************************************************/
      temp = new double[diffSigLength];
      
      for(j = 0; j < diffSigLength; j++)
      {
        // if you did your math correctly, 
        // diffSigDec should be equal to bandSigDec[difflen/2 
        // why difflen/2, only the second half of
        
        // first diffSig is banksig(difflen + bottom:up -
        // bankSig(bottom:up)
        // doing difflen/2 is sort of shifting into the
        // signal by half a difflen length (probly because
        // its boring and doesnt do anything for that interval
        // and thanks to the filtering and would corrupt
        // the maxreldiff caluclation!
        //	      temp[j] = diffSig[j]/bandSigDec[(int)(floor(difflen/2))
        //                             + bottom + j]; 
        temp[j] = diffSig[j]/bandSigDec[(int)(floor(difflen)) + bottom + j]; 
      }
      
      // okay get the maxIndex from temp
      // IO performance issues here 2 for loops instead of one?
      maxRelativeDiffIndex = getMaximumIndex(temp, diffSigLength);
      
      // convert the maxrelid into a sample value, i.e. undo the
      // effect of the decimation on the original signal
      // IO comment do i want to put the point halfway up the slope?
      // or what, right now we're @ the base of the slope.
      maxRelativeDiffIndex = bottom + ceil(maxRelativeDiffIndex); 
      
      // add data to peaktimes . . .
      /*      peaktimesB[*peakCount] = 
              decimation_step * maxRelativeDiffIndex + ceil(decimation_step/2); */
      peaktimesB.push_back(decimation_step * maxRelativeDiffIndex +
                           ceil(decimation_step/2));
      

      // peaktimesB.push_back(maxRelativeDiffIndex +
      //   ceil(decimation_step/2));

      //cout << " peaktimesB[peakCount] = " <<  peaktimesB[*peakCount]
      //     << endl;
      
      // i don't think i need to keep this puppy around anymore . . .
      (*peakCount)++;
      
      // clean up potential memory leaks
      delete [] diffSig;
      delete [] temp;
      delete [] riseIds;
      delete [] customGaussEnv; 
    }
    
  }  
  
  delete surfboard;
  delete gaussEnvelope;
  delete bandSigDec;
}


/**********************************************************************/
/* getPrunedOnsets returns the indexes of the times and loud vectors  */
/* that are within the desired limits, while discarding others        */
/* returned will the the matlab(+1) indices corresponding to the      */
/* accepted onsets                                                    */
/**********************************************************************/
void BandOnsets::getPrunedOnsets(double *times,
                                 double *louds,
                                 int onsetCount,
                                 double loudthresh,
                                 double mininterv,
                                 DoubleVector &temp_acceptedIndexes)
{
  int i;
  int j;
  int belowOK = 0;
  int aboveOK = 0;
  int dev;
  
  /* iterate thru vectors */
  for(i = 0; i < onsetCount - 1; i++)
  {
    
    if(louds[i+1] >= loudthresh)
    {
      belowOK = 1;
      dev = 1;
      
      // check onsets before current one 
      while((i - dev) >= 0 && 
            (times[i + 1] - times[i - dev + 1]) < mininterv)
      {
        if(louds[i + 1] < louds[i - dev + 1])
        {
          belowOK = 0;
        }
        dev++;
      }
      
      // if times[i+1] < time[i - dev]
      // then belowOK is set to zero
      // this checks for all guys less than times[i] within
      // the mininterval       

      // an equivalent operation is performed on the 
      // onsets "above" the current times[i]
      
      aboveOK = 1;
      dev = 1;
      
      while((i + dev) < onsetCount &&
            (times[i + dev + 1] - times[i + 1]) < mininterv )
      {
        
        if(louds[i + 1] <= louds[i + dev + 1])
        {
          aboveOK = 0;
        }
        dev = dev + 1;
      }
      
      /* accept the current onset only if things are kosher */
      /* below and above it                                 */
      if((aboveOK == 1) && (belowOK == 1))
      {
        /* add 2 because adding one just gets us the non-MATLAB */
        /* indicies which start @ 1 */
        temp_acceptedIndexes.push_back(i+2);
      }
    }  // end if
  } // end for
}


