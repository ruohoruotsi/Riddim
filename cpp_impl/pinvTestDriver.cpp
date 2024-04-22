#include <iostream>

#include <math.h>
#include <string.h>

// MAC console
#include <console.h>
#include <SIOUX.h>

// #include "pinv.h"      // psuedoinverse code
#include "dspUtils.h"  

#include "tnt/tnt.h"   // Template Numerical Toolkit
#include "tnt/vec.h"
#include "tnt/cmat.h"

using namespace TNT;

int main(int argc, char* argv[])
{		

 	// essential for MAC OS console applications
	argc = ccommand(&argv);
	
	int numComponents = 3;
	int i,j;
	
	// get data ready for action
	double* aa = new double [numComponents*numComponents];
	
	aa[0] = .4277;   aa[1] = 2.3197;  aa[2] = 4.8508; 
	aa[3] = 1.5465;  aa[4] = 5.8849;  aa[5] = 4.7043; 
	aa[6] = 2.9105;  aa[7] = 4.2339;  aa[8] = -11.1307; 
	
  cout << aa[0] << " " << aa[1] << " " << aa[2] << endl;
  cout << aa[3] << " " << aa[4] << " " << aa[5] << endl;
  cout << aa[6] << " " << aa[7] << " " << aa[8] << endl << endl;
  
  Matrix<double> A(3, 3, aa);
  Matrix<double> COVA = A*transpose(A);
  
  double *tt = COVA.getMat1D();
  
  // call psuedoinverse
	// double *tt = psuedoinverse(aa, numComponents, numComponents, 1e-12);
	
	cout << tt[0] << " " << tt[1] << " " << tt[2] << endl;
  cout << tt[3] << " " << tt[4] << " " << tt[5] << endl;
  cout << tt[6] << " " << tt[7] << " " << tt[8] << endl;
    
	  
	return 0;
	
}