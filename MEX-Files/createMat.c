/* $Revision: 1.5 $ */
/*
 * MAT-file creation program
 *
 * See the MATLAB API Guide for compiling information.
 *
 * Calling syntax:
 *
 *   matcreat
 *
 * Create a MAT-file which can be loaded into MATLAB.
 *
 * This program demonstrates the use of the following functions:
 *
 *  matClose
 *  matGetArray
 *  matOpen
 *  matPutArray
 *  matPutArrayAsGlobal
 *
 * Copyright (c) 1984-98 by The MathWorks, Inc.
 * All Rights Reserved.
 */

#include <stdio.h>

#include "mat.h"

int createMAT(const char *file, double *data, int datalen) 
{
  MATFile *pmat;
  mxArray *pa1; 
  
  printf("Creating a mutherfucking file %s...\n\n", file);
  pmat = matOpen(file, "w");
  if (pmat == NULL) 
    {
      printf("Error creating file %s\n", file);
      printf("(do you have write permission in this directory?)\n");
      return(1);
    }
  
  pa1 = mxCreateDoubleMatrix(1,datalen,mxREAL);
  mxSetName(pa1, "LocalDouble");

  memcpy((char *)(mxGetPr(pa1)), (char *)data, 1*datalen*sizeof(double));
  matPutArray(pmat, pa1);

  /* clean up */
  mxDestroyArray(pa1);

  
  if (matClose(pmat) != 0) {
    printf("Error closing file %s\n",file);
    return(1);
  }

  printf("Done\n");
  return(0);
}


