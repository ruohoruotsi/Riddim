/*                                          */
/* iroro header to save timing info as MIDI */
/*                                          */

myputc(c); 

int writeTrack(int track);
void saveMIDI(double* times, 
              int timeLength,
              double* volume, 
              char* filename);
