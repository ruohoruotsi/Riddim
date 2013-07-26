#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "midifile.h"


/* routine to write characters to a file  */

static double* times;
static int*    timesLength;
static double* volume;

FILE *fp;
myputc(c) { return(putc(c,fp));}


/*************************************************************/
/* necessary function defined to implement the MIDI write    */
/*************************************************************/
int writeTrack(int track)
{
  int i;
  double numticks;
  
  char data[2];
  
  /* 120 beats/per/second */
  mf_write_tempo((long)500000);
  

  /*************************************************************/
  /*************************************************************/
  /*              write zeroth midi note                       */
  /*************************************************************/
  /*************************************************************/
  data[0] = 60;        /* constant note number, middle C */
  data[1] = volume[0];  /* velocity is derived from volume */
  
  /*  write note on event                 */
  /*  delta,midimsg, chan, data, datasize */
  if(!mf_write_midi_event(0,note_on,1,data,2))
    return(-1);
  
  /*  write note off event                */
  /*  delta,midimsg, chan, data, datasize */
  if(!mf_write_midi_event(48,note_off,1,data,2))
    return(1);
  


  /*************************************************************/
  /*************************************************************/
  /*              write middle midi notes                      */
  /*************************************************************/
  /*************************************************************/
  for(i = 1 ; i < timesLength; i++)
  {
    data[0] = 60;  /* constant note number, middle C */
    data[1] = volume[i];  /* velocity is derived from volume */

    // 480 PPQN 4QN == 1 second
    numticks = (times[i] - times[i-1])*480*4 - 48;

    /*  write note on event   */
    if(!mf_write_midi_event((unsigned long)numticks,    // delta time since 
                            note_on,  // MIDI message
                            1,        // MIDI channel
                            data,     // data (vel, nn)
                            2))       // size of data in bytes
      return(-1);

    // 48 is the minimum number of ticks possible 
    // highest temporal resolution    
    /*  write note off event   */
    if(!mf_write_midi_event(48,        // delta time since note on
                            note_off,  // MIDI message
                            1,         // MIDI channel
                            data,      // data (vel, nn)
                            2))        // size of data in bytes
      return(-1);
  }
  
  return(1);
} /* end of write_track() */


/*************************************************************/
/* interface method called to write a vector of sample times */
/* to a midi file.                                           */
/*************************************************************/
void saveMIDI(double* _times, 
              int     _timesLength,
              double* _volume, 
              char* filename)
{
  int i;
  timesLength = _timesLength;
  
  // initialize static variable
  times = (double*)calloc((timesLength), sizeof(double));
  volume = (double*)calloc((timesLength), sizeof(double));
  for(i = 0; i < timesLength; i++)
  {
    times[i] = _times[i];
    volume[i] = _volume[i];
  }

  /*  
      mexPrintf("times[0] = %f\n", times[0]);
      mexPrintf("times[1] = %f\n", times[1]);
      
      mexPrintf("volume[0] = %f\n", volume[0]);
      mexPrintf("volume[1] = %f\n", volume[1]);
  */
  

  /******************************************************/
  /*   open up a new file to write to                   */
  /******************************************************/
  if((fp = fopen(filename,"w")) == 0L)
  {
    exit(1);
  }

  
  /******************************************************/
  /* iroro defined functions that are needed by mfwrite */
  /******************************************************/
  Mf_putc = myputc;
  Mf_writetrack = writeTrack;

  
  /******************************************************/
  /* write a single MIDI track of Type 0                */
  /******************************************************/
  
  mfwrite(0,    // format
          1,    // number of tracks
          480,  // division
          fp);  // file to write to
  
  free(times);
  free(volume);
  
  // close the file 
  fclose(fp);

  return(0);
}

