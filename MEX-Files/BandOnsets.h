/**********************************************************************
 *                                                                    *
 * bandOnsets.h                                                       *
 *                                                                    *
 * Computational function that calculates the onsets                  *
 * that occur in a specific band                                      *
 *                                                                    *
 **********************************************************************/
#include <vector>

using namespace std;

typedef vector<double, allocator<double> > DoubleVector;

class BandOnsets
{
 public:
  BandOnsets();
  ~BandOnsets();

  void getBandOnsets(DoubleVector &peaktimesB,             // output args
                     DoubleVector &peakpressesB,     
                     DoubleVector &peakBgPressesB,   
                     DoubleVector &bankPressesB,   
                     int *peakCount,                        
                     double *signal,  // input args
                     double srate, double timefrac, 
                     double loudtimefrac, 
                     int diffPrecision,
                     double Fdec, double *Bdec, double *Adec,
                     int signalLength,
                     double thresholdValue); 
  

  void getPrunedOnsets(double *times,
                       double *louds,
                       int onsetCount,
                       double loudthresh,
                       double mininterv,
                       DoubleVector &temp_acceptedIndexes);
  
};

