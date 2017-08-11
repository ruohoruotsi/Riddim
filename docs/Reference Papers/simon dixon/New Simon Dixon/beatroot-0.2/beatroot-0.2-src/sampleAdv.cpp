//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sampleAdv.cpp
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

#include "sample.h"
#include "util.h"


sample* sample :: downSample(int filterFreq, int newRate) {
	double freq0[] = {		// cfilter(4410, 44100)       
		4410.0000000000, 44100.0000000000 };
	double a0[] = {		// created by Matlab: [a,b,c,d] = ellip(6, 3, 50, 0.2);
        0.7572705450, -0.2340837208, -0.0000000000, -0.0000000000,
        0.0000000000, -0.0000000000, 0.2340837208, 0.9688180124,
        0.0000000000, 0.0000000000, -0.0000000000, 0.0000000000,
        1.4830530047, 14.0781914396, 0.7755540636, -0.4848005317,
        -0.0000000000, -0.0000000000, 0.4049355071, 3.8439351607,
        0.4848005317, 0.8676291754, -0.0000000000, 0.0000000000,
        1.5967077814, 15.1570832262, 0.1180164489, 0.7952913266,
        0.7936084093, -0.5770638363, 0.5137143164, 4.8765408035,
        0.0379698402, 0.2558718288, 0.5770638363, 0.8143392563 };
	double b0[] = {
        0.8074760798, 0.1075628370, 1.4973489817, 0.4088389069,
        1.6120993404, 0.5186662959 };
	double c0[] = {
        0.0062645229, 0.0594672962, 0.0004630257, 0.0031202458,
        0.0001565677, 0.0007394417 };
	double d0[] = {
        0.0063249101 };
	double freq1[] = {		// cfilter(10000, 44100)
		10000.0000000000, 44100.0000000000 };
	double a1[] = {
        0.3837282119, -0.4899562485, -0.0000000000, -0.0000000000,
        -0.0000000000, 0.0000000000, 0.4899562485, 0.8265142508,
        0.0000000000, 0.0000000000, 0.0000000000, -0.0000000000,
        6.0245353743, 23.5447644891, 0.2014939927, -0.8720165214,
        0.0000000000, -0.0000000000, 4.3724682869, 17.0882449276,
        0.8720165214, 0.3671105988, 0.0000000000, -0.0000000000,
        8.2630134648, 32.2930639313, 0.5129109845, 1.0196855241,
        0.1350104605, -0.9706664387, 7.0665691038, 27.6171845557,
        0.4386439562, 0.8720399949, 0.9706664387, 0.1698813641 };
	double b1[] = {
        1.6901130480, 0.5984422674, 8.8260154079, 6.4057176318,
        12.1054122226, 10.3526071167 };
	double c1[] = {
        0.0213065130, 0.0832689658, 0.0013225616, 0.0026293002,
        0.0003381921, 0.0005005913 };
	double d1[] = {
        0.0312142929 };
	double freq2[] = {		// cfilter(5000, 44100)
		5000.0000000000, 44100.0000000000 };
	double a2[] = {
        0.7224805444, -0.2627377738, -0.0000000000, -0.0000000000,
        -0.0000000000, -0.0000000000, 0.2627377738, 0.9599234151,
        0.0000000000, 0.0000000000, 0.0000000000, 0.0000000000,
        1.8852500652, 15.6252529862, 0.7284091852, -0.5403950478,
        0.0000000000, 0.0000000000, 0.5894320673, 4.8853069096,
        0.5403950478, 0.8310430133, 0.0000000000, 0.0000000000,
        2.0722526031, 17.1751598218, 0.1591515160, 0.8698604163,
        0.7407027787, -0.6412929386, 0.7634393290, 6.3275068252,
        0.0586330674, 0.3204655898, 0.6412929386, 0.7637410372 };
	double b2[] = {
        0.9063187602, 0.1382449132, 1.9014015468, 0.5944819020,
        2.0900061894, 0.7699799341 };
	double c2[] = {
        0.0073722976, 0.0611027775, 0.0005662014, 0.0030946371,
        0.0001840360, 0.0007220140 };
	double d2[] = {
        0.0074354582 };
	double filt = double(filterFreq) / double(rate());
	if (fabs(freq1[0] / freq1[1] - filt) < 0.01)
		applyFilter(a1,b1,c1,d1,6);
	else if (fabs(freq2[0] / freq2[1] - filt) < 0.01)
		applyFilter(a2,b2,c2,d2,6);
	else if (fabs(freq0[0] / freq0[1] - filt) < 0.01)
		applyFilter(a0,b0,c0,d0,6);
	else
		assertWarning(FALSE, "downsampling filter not found");
	return changeRate(newRate);
} // downSample()

void mult(double* a, int aRows, int aCols, double *b, int bCols, double *c) {
// Performs matrix multiplication c = a * b
	for (int row = 0; row < aRows; row++)
		for (int col = 0; col < bCols; col++, c++) {
			*c = 0.0;
			for (int inner = 0; inner < aCols; inner++)
				*c += a[row * aCols + inner] * b[inner * bCols + col];
		}
} // mult()

void sample :: applyFilter(double* a, double *b, double* c, double* d, int n) {
// Filters the sample using state-space equations:
//	s(t+1) = a[nxn] * s(t)[nx1] + b[nx1] * x(t)
//	y(t)   = c[1xn] * s(t)[nx1] + d[1x1] * x(t)
	double state[n], temp[n];
	double input;
	int i;
	if (!assertWarning(channels() == 1, "Cannot filter multi-channel data"))
		return;
	for (i = 0; i < n; i++)
		state[i] = 0.0;
	for (i = 0; i < audioSize(); i++) {
		input = (double) data[i];
		mult(c, 1, n, state, 1, temp);
		data[i] = audioLimit(int(*temp + *d * input));
		mult(a, n, n, state, 1, temp);
		for (int j = 0; j < n; j++)
			state[j] = temp[j] + b[j] * input;
	}
} // applyFilter()

// Linear crossfade of 2 equal length samples
sample* sample :: crossfade(const sample& t) const {
	if (!assertWarning(rate() == t.rate(), "crossfade: different rates") ||
		!assertWarning(channels()==t.channels(), "crossfade: channel counts")||
		!assertWarning(length() == t.length(), "crossfade: different lengths"))
		return NULL;
	sample* s3 = new sample(length(), t.rate());
	if (!assertWarning(s3->data != NULL, "Unable to create audio sample")) {
		delete s3;
		return NULL;
	}
	const audioSample* p1 = data;
	const audioSample* p2 = t.data;
	audioSample* p3 = s3->data;
	double frac;
	int len = length();
	double ln = double(len);
	int val;

	for (int i=0; i < len; i++) {
		frac = double(i) / ln;
		val =  (int) (double(*p1++) * (1 - frac) + double(*p2++) * frac );
		*p3++ = audioLimit(val);
	}
	return s3;
} // crossfade()

sample* sample :: interpolateRate(int newRate) const {
	double ratio = (double) rate() / (double) newRate;
	int len = (int)floor((double) length() / ratio);
	sample* out = new sample(len, rate());
	if (!assertWarning(out->data != NULL, "Unable to create audio sample")) {
		delete out;
		return NULL;
	}
	double t = 0;
	int index;
	double delta;
	int sample_value;
	for (int i=0; i<len; i++, t += ratio) {
		index = (int)floor(t);
		delta = t - (double)index;
		sample_value = (int)rint((1-delta)*data[index] + delta*data[index+1]);
		out->data[i] = audioLimit(sample_value);
	}
	return out;
} // interpolateRate()

sample* sample ::
changeRate(int newRate, int start, int ilength, int LIMIT) const {
	if (!assertWarning(start >= 0, "changeRate(): ignoring illegal start"))
		start = 0;
	if (!assertWarning(start < length(),"changeRate(): ignoring illegal start"))
		start = 0;
	if ((ilength < 0) || (start + ilength > length()))
		ilength = length() - start;
	double ratio = (double) rate() / (double) newRate;
	int len = (int)rint(double(ilength) / ratio);
	sample* out = new sample(len, newRate);
	if (!assertWarning(out->data != NULL, "Unable to create audio sample")) {
		delete out;
		return NULL;
	}
	double time = ilength - len * ratio;
	double y;
	int index,sample_value,begin,stop,i;
	if (time < 0) {
		out->data[0] = (audioSample) ( (double)data[start] * time );
		i = 1;
		time += ratio;
	} else
		i = 0;
	for ( ; i<len; i++, time += ratio) {
		if (abs(time-rint(time)) < 5e-10)
			out->data[i] = data[start+(int)rint(time)];
		else {
			begin = (int)floor(time-LIMIT);
			if (begin < 0) begin = 0;
			stop = (int)ceil(time+LIMIT);
			if (stop >= length()) stop = length();
			y = 0;
			for (index = begin; index < stop; index += 2) {
				y += (double)data[start+index]/(time-(double)index) -
						(double)data[start+index+1]/(time-(double)index-1);
			}
			sample_value = (int)rint(y * sin(M_PI*(time-(double)begin))/ M_PI);
			out->data[i] = audioLimit(sample_value);
		}
	}
	return out;
}

sample* sample ::
warpRate(int initialRate, int LIMIT) const {
	double t1 = 1 / (double) initialRate;
	double t2 = 1 / (double) rate();
	int len = (int) rint((2*length()*t2 - t1 + t2) / (t1 + t2));
	sample* out = new sample(len, rate());
	if (!assertWarning(out->data != NULL, "Unable to create audio sample")) {
		delete out;
		return NULL;
	}
	out->data[0] = data[0];
	double inc1 = t1/t2;
	double inc2 = (t2-t1)/((double)len*t2);
	double time = 0;
	int index;
	double y;
	int sample_value,start,stop;
	for (int i=1; i<len; i++) {
		inc1 += inc2;
		time += inc1;

		if (abs(time-rint(time)) < 5e-10)
			out->data[i] = data[(int)rint(time)];
		else {
			start = (int)floor(time-LIMIT);
			if (start < 0) start = 0;
			stop = (int)ceil(time+LIMIT);
			if (stop >= length()) stop = length();
			y = 0;
			for (index = start; index < stop; index += 2) {
				y += (double)data[index]/(time-(double)index) -
						(double)data[index+1]/(time-(double)index-1);
			}
			sample_value = (int)rint(y * sin(M_PI*(time-(double)start))/ M_PI);
			out->data[i] = audioLimit(sample_value);
		}
	}
	return out;
}
