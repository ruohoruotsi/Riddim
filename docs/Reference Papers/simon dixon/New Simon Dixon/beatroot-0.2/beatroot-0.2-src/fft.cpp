//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: fft.cpp
// 
//  Copyright (C) 2001  Simon Dixon <simon@oefai.at>
// 
//  This file is part of BeatRoot.
//
//  BeatRoot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  BeatRoot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with BeatRoot; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "fft.h"
#include "sample.h"
#include "util.h"

//	Window functions as found in [Harris 1978: Proc. IEEE, 66, 1, 51-83]

void makeHamming(double* data, int size) {
	double scale = 1.0 / (double)size / 0.54; // Normalise by N & Coherent_Gain
	double factor = 2 * M_PI / (double)size;
	for (int i = 0; i < size; i++)
		data[i] = scale * ( 25.0/46.0 - 21.0/46.0 * cos(factor * (double)i));
}

// minimum 4-sample Blackman-Harris
void makeBlackmanHarris4sMin(double* data, int size) {
	double scale = 1.0 / (double)size / 0.36; // Normalise by N & Coherent_Gain
	for (int i = 0; i < size; i++)
		data[i] = scale * ( 0.35875 -
							0.48829 * cos(2*M_PI*(double)i/(double)size) +
							0.14128 * cos(4*M_PI*(double)i/(double)size) -
							0.01168 * cos(6*M_PI*(double)i/(double)size));
}

// 74-dB 4-sample Blackman-Harris window
void makeBlackmanHarris4s(double* data, int size) {
	double scale = 1.0 / (double)size / 0.4; // Normalise by N & Coherent_Gain
	for (int i = 0; i < size; i++)
		data[i] = scale * ( 0.40217 -
							0.49703 * cos(2*M_PI*(double)i/(double)size) +
							0.09392 * cos(4*M_PI*(double)i/(double)size) -
							0.00183 * cos(6*M_PI*(double)i/(double)size));
}

// minimum 3-sample Blackman-Harris
void makeBlackmanHarris3sMin(double* data, int size) {
	double scale = 1.0 / (double) size / 0.42; // Normalise by N & Coherent_Gain
	for (int i = 0; i < size; i++)
		data[i] = scale * ( 0.42323 -
							0.49755 * cos(2*M_PI*(double)i/(double)size) +
							0.07922 * cos(4*M_PI*(double)i/(double)size));
}

// 61-dB 3-sample Blackman-Harris window
void makeBlackmanHarris3s(double* data, int size) {
	double scale = 1.0 / (double) size / 0.45; // Normalise by N & Coherent_Gain
	for (int i = 0; i < size; i++)
		data[i] = scale * ( 0.44959 -
							0.49364 * cos(2*M_PI*(double)i/(double)size) +
							0.05677 * cos(4*M_PI*(double)i/(double)size));
}

void makeGauss(double* data, int size) {  // somewhat between 61/3 and 74/4 BHW
	double delta = 5.0 / double(size);
	double x = double(1 - size) / 2.0 * delta;
	double c = -M_PI * exp(1.0) / 10.0;
	double sum = 0;
	for (int i = 0; i < size; i++) {
		data[i] = exp(c * x * x);
		x += delta;
		sum += data[i];
	}
	for (int i = 0; i < size; i++)
		data[i] /= sum;
}

void makeRectangle(double* data, int size) {
	for (int i = 0; i < size; i++)
		data[i] = 1.0 / (double) size;
}

double* windowChoice(int choice, int size, int support) {
	if (!assertWarning(support <= size, "Illegal parameters: support > size"))
		support = size;
	double* data = new double[size];
	double* ptr = data + (size - support) / 2;
	memset(data, 0, size * sizeof(double));
	switch (choice) {
		case RECT:		makeRectangle(ptr, support); break;
		case HAMMING:	makeHamming(ptr, support); break;
		case BH3:		makeBlackmanHarris3s(ptr, support); break;
		case BH4:		makeBlackmanHarris4s(ptr, support); break;
		case BH3MIN:	makeBlackmanHarris3sMin(ptr, support); break;
		case BH4MIN:	makeBlackmanHarris4sMin(ptr, support); break;
		case GAUSS:		makeGauss(ptr, support); break;
		default:		makeRectangle(ptr, support); break;
	}
	return data;
} // windowChoice()

void applyWindow(double* data, double* window, int size) {
	for (int i = 0; i < size; i++)
		data[i] *= window[i];
}

// Complex conjugate
void complex_cnjg(complex *r, complex *z) {
    r->r = z->r;
    r->i = -z->i;
}

// Complex exponential to real+imag representation
void polar2rect(complex *r, complex *z) {
    double expx = exp(z->r);
    r->r = expx * cos(z->i);
    r->i = expx * sin(z->i);
} // polar2rect()

// Fast Fourier Transform of n complex data points, where n is a power of 2
//	direction = -1 for forward transform, +1 for inverse.
void complexFFT(complex *data, int size, int direction) {
    int i, l, m, mr,tmp_int;
    complex t, tmp2, tmp;

    double pisign = (double)direction * M_PI;
    mr = 0;
    for (m = 1 ; m < size ; ++m) {
		l = size / 2;
		while (mr + l >= size)
			l /= 2;
		mr = mr % l + l;
		if (mr > m) {
			t.r = data[m].r;
			t.i = data[m].i;
			data[m].r = data[mr].r;
			data[m].i = data[mr].i;
			data[mr].r = t.r;
			data[mr].i = t.i;
		}
    }
    l = 1;
    while (l < size) {
		for (m = 0 ; m < l ; ++m) {
			tmp_int = l * 2;
			for (i = m; tmp_int < 0 ? i >= (size - 1): i < size; i += tmp_int) {
				tmp.r = 0.0;
				tmp.i = (double) m * pisign / (double) l;
				polar2rect(&tmp2, &tmp);
				t.r = data[i+l].r * tmp2.r - data[i+l].i * tmp2.i;
				t.i = data[i+l].r * tmp2.i + data[i+l].i * tmp2.r;
				data[i+l].r = data[i].r - t.r;
				data[i+l].i = data[i].i - t.i;
				data[i].r += t.r;
				data[i].i += t.i;
			}
        }
        l *= 2;
    }
} // complexFFT()

// FFT routine for real input data (length n = power of 2)
// Must have space for 2 more values on return (i.e. size = n + 2)
void fft(double* inputData, int size) {
	complex* data = (complex*) inputData;	// coerce to complex array
    complex u, tmp1, tmp2;
    double tpn, dtmp;

    tpn = 2.0 * M_PI / (double) size;
    complexFFT(data, size / 2, -1);
    data[size/2].r = data[0].r;
    data[size/2].i = data[0].i;
    for (int m = 0 ; m <= (size / 4) ; ++m) {
		u.r = sin((double) m * tpn);
		u.i = cos((double) m * tpn);
		complex_cnjg(&tmp2, &data[size / 2 - m]);
		tmp1.r = (((1.0 + u.r) * data[m].r - u.i * data[m].i)
					+ (1.0 - u.r) * tmp2.r - -u.i * tmp2.i) / 2.0;
		tmp1.i = (((1.0 + u.r) * data[m].i + u.i * data[m].r)
					+ (1.0 - u.r) * tmp2.i + -u.i * tmp2.r) / 2.0;
		dtmp = ((1.0 - u.r) * data[m].r - -u.i * data[m].i
					+ (1.0 + u.r) * tmp2.r - u.i * tmp2.i) / 2.0;
		data[m].i = ((1.0 - u.r) * data[m].i + -u.i * data[m].r
					+ (1.0 + u.r) * tmp2.i + u.i * tmp2.r) / 2.0;
		data[m].r = dtmp;
		complex_cnjg(&data[size / 2 - m], &tmp1);
	}
} // fft()

// Computes a single DFT component of a real vector
//	where freq is relative to sampling frequency
complex dftComponent(double* data, int size, double freq) {
    complex solution, tmp1, tmp2;
    double rad = 2.0 * M_PI * freq;
    solution.r = data[0];
    solution.i = 0.0;
    for (int k = 1 ; k < size ; k++) {
		tmp1.r = 0.0;
		tmp1.i = (double) -k * rad;
		polar2rect(&tmp2, &tmp1);
		solution.r += data[k] * tmp2.r;
		solution.i += data[k] * tmp2.i;
    }
    return solution ;
} // dftComponent()
