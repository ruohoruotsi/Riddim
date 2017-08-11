//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: onset.cpp
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

#include "event.h"
#include "includes.h"
#include "param.h"
#include "sample.h"
#include "spectrum.h"
#include "util.h"

#define AMP_PEAK 1
#define SLOPE_PEAK 2
#define F_PEAK_PEAK 4
#define F_S_PEAK_PEAK 8
#define ONSET 16
#define REAL_ONSET 32

void makeAmplitudeEnvelope(audioSample* ptr, int length, int slide, // input
						   int*& env, int& sz, double& average);	// output

void findPeaks( int* base,          // data array
				int length,         // length of data array
				int peakWidth,      // ignore peaks as close as this
				int threshold,      // ignore peaks below this level
				int* peaks,         // array for output
				int& peakLength);

// Uses an n-point linear regression to estimate the slope of data
void getSlope(int* data, int sz, int n, int* slope, double& average) {
	int i = 0, sx = 0, sxx = 0, sy = 0, sxy = 0;
	for ( ; i < n; i++) {
		sx += i;
		sxx += i * i;
		sy += data[i];
		sxy += i * data[i];
	}
	ift delta = 4 * sxx - sx * sx;
	average = sy;
	for (i = 0; i < sz - n; i++) {
		slope[i] = (4 * sxy - sx * sy) / delta;
		sy += data[i + n] - data[i];
		sxy += n * data[i + n] - sy;
		average += data[i];
	}
	slope[i] = (4 * sxy - sx * sy) / delta;
	for ( ; i < sz; i++)
		slope[i] = 0;
	average /= double(sz);
} // getSlope()

int getAlignment(int* data, int sz, int* onset) {
	int score[11] = {0,0,0,0,0,0,0,0,0,0,0};
	for (int delta = -5; delta <= 5; delta++) {
		for (int i = 0; i < sz; i++)
			if ((onset[i]) && (i + delta >= 0) && (i + delta < sz))
				score[delta+5] += data[i+delta];
	// 	cout << setw(6) << score[delta+5] << " ";
	}
	int max = 0;
	for (int i = 1; i < 11; i++)
		if (score[i] > score[max])
			max = i;
	max -= 5;
	// cout << "   Adjustment = " << max << endl;
	return max;
} // getAlignment()

void shift(int* data, int sz, int offset) {
	if (offset < 0) {
		for (int i = sz - 1; i >= -offset; i--)
			data[i] = data[i+offset];
		for (int i = 0; i < -offset; i++)
			data[i] = 0;
	} else {
		for (int i = 0; i < sz - offset; i++)
			data[i] = data[i+offset];
		for (int i = sz - offset; i < sz; i++)
			data[i] = 0;
	}
} // shift()

void bitcopy(int src, int& dest, int mask) {
	if (src & mask)
		dest |= mask;
	else if (dest & mask)
		dest -= mask;
} // bitcopy()

int pGetAlignment(int* data, int mask, int sz, int* onset) {
	int score[11] = {0,0,0,0,0,0,0,0,0,0,0};
	for (int delta = -5; delta <= 5; delta++) {
		for (int i = 0; i < sz; i++)
			if ((onset[i]) && (i + delta >= 0) && (i + delta < sz) &&
					(data[i+delta] & mask))
				score[delta+5]++;
	// 	cout << setw(6) << score[delta+5] << " ";
	}
	int max = 0;
	for (int i = 1; i < 11; i++)
		if (score[i] > score[max])
			max = i;
	max -= 5;
	// cout << "   Adjustment = " << max << endl;
	return max;
} // pGetAlignment()

void pShift(int* data, int mask, int sz, int offset) {
	if (offset < 0) {
		for (int i = sz - 1; i >= -offset; i--)
			bitcopy(data[i+offset], data[i], mask);
		for (int i = 0; i < -offset; i++)
			bitcopy(0, data[i], mask);
	} else {
		for (int i = 0; i < sz - offset; i++)
			bitcopy(data[i+offset], data[i], mask);
		for (int i = sz - offset; i < sz; i++)
			bitcopy(0, data[i], mask);
	}
} // pShift()


class OnsetLearner {
		const char* fileName;	// audio data file name
		char* midiName;			// MIDI data file name
		double* realOnsets;		// array of onset times of notes
		int realCount;			// length of realOnsets[]
		double* predictedOnsets;// array of predicted onset times of notes
		int predictedCount;		// length of predictedOnsets[]
		double audioLength;		// total length of audio data to be processed
		int chunkSize;			// size of audio chunk; whole number of seconds
		int maxOnsetRate;		// max onsets per second
		double deltaTime;		// time resolution
		double* rowData;
		int rows;				// total number of (deltaTime) time units
		int cols;				// number of data fields in rowData
		int end;				// number of rows used in rowData

	public:
		OnsetLearner(const char* name);
		void processChunk(double st, double ln);
		void predictOnsets(int* onsets, int& count, double* weights);
		void printLearningData();
		double evaluate();
};


void OnsetLearner :: processChunk(double st, double ln) {
	sample inData(fileName, st, st + ln, true);
	int slide = int(deltaTime * inData.rate());
	int i, j, sz;
	int* env;
	double average;
	makeAmplitudeEnvelope(inData.data, inData.length(), slide, env, sz,average);
	int peaks[sz];
	int ampPeakCount = sz;
	double thresholdFactor = parameters->getDouble("ampThresholdFactor", 10.0);
	int pw = parameters->getInt("peakWidth", 5);
	findPeaks(env, sz, pw, int(average/thresholdFactor), peaks, ampPeakCount);
	int properties[sz];
	memset(properties, 0, sz * sizeof(int));
	for (j = 0; j < ampPeakCount; j++)
		properties[peaks[j]] |= AMP_PEAK;

	int slope[sz];
	getSlope(env, sz, 4, slope, thresholdFactor); // using 4-pt lin regression
	int pCounter = sz;
	thresholdFactor /= parameters->getDouble("slopeThresholdFactor",10.0);
	findPeaks(slope, sz-4, pw, int(thresholdFactor), peaks, pCounter);
	for (j = 0; j < pCounter; j++)
		properties[peaks[j]] |= SLOPE_PEAK;

	// Do frequency domain analysis
	// Changes 12/10/2001(pm): take log of power and sum log powers & slopes
	//  instead of counting across peaks in different frequency bands,
	//  and changing p[4],p[6] in predictOnsets()
	sample* s = inData.downSample(5000,12000);
	double freqSep = parameters->getDouble("freqSep", 25.0);
	int wsize = 1 << int(ceil(log( double(s->rate()) / freqSep ) / log(2)));
	int wslide = int(rint(parameters->getDouble("windowSlide",0.01) *
					(double)(s->rate())));
	int timeCount = (s->length() - wsize + wslide - 1) / wslide;
	int freqCount = wsize / 2;
	int* FDdata = new int[freqCount*timeCount];
	spectrum* frame = new spectrum(s, wsize, wsize, wslide);
	int amp[sz];
	for (i = 0; i < timeCount; i++, frame->nextFrame()) {
		for (j = 0; j < freqCount; j++)
//			FDdata[j * timeCount + i] = int(frame->powerComponent(j)) >> 10;
			FDdata[j * timeCount + i] = int(20*log(frame->powerComponent(j)));
		amp[i] = int(frame->getRMSAmplitude());
	}
	for ( ; i < sz; i++) {
		for (j = 0; j < freqCount; j++)
			FDdata[j * timeCount + i] = 0;
		amp[i] = 0;
	}
	int fPeakCount[sz];
	memset(fPeakCount, 0, sz * sizeof(int));
	int fSlopePeakCount[sz];
	memset(fSlopePeakCount, 0, sz * sizeof(int));
	for (i = 0; i < freqCount; i++) {
		pCounter = sz;
		findPeaks(&FDdata[i*timeCount], sz, pw, 0, peaks, pCounter);
		for (j = 0; j < pCounter; j++)
//			if ((FDdata[i * timeCount + peaks[j]] > 50) &&
//					(amp[peaks[j]] / FDdata[i * timeCount + peaks[j]] < 100))
//			fPeakCount[peaks[j]]++;
			fPeakCount[peaks[j]] += max(FDdata + i * timeCount, sz,
										peaks[j] - 2, peaks[j] + 5);
		int fSlope[sz];
		getSlope(&FDdata[i*timeCount], sz, 4, fSlope, thresholdFactor);
		pCounter = sz;
		thresholdFactor /= parameters->getDouble("fSlopeThresholdFactor",10.0);
		findPeaks(fSlope, sz-4, pw, int(thresholdFactor), peaks,pCounter);
		for (j = 0; j < pCounter; j++)
//			fSlopePeakCount[peaks[j]]++;
			fSlopePeakCount[peaks[j]] += max(fSlope, sz-4,
											 peaks[j] - 2, peaks[j] + 5);
	}
	pCounter = sz;
	findPeaks(fPeakCount, sz, pw, 0, peaks, pCounter);
	for (j = 0; j < pCounter; j++)
		properties[peaks[j]] |= F_PEAK_PEAK;
	pCounter = sz;
	findPeaks(fSlopePeakCount, sz, pw, 0, peaks, pCounter);
	for (j = 0; j < pCounter; j++)
		properties[peaks[j]] |= F_S_PEAK_PEAK;
	// int adjustment = getAlignment(env, sz, onset);
	int adjustment = 2;
	shift(env, sz, adjustment);
	// adjustment = pGetAlignment(properties, AMP_PEAK, sz, onset);
	pShift(properties, AMP_PEAK, sz, adjustment);
	// adjustment = getAlignment(slope, sz, onset);
	adjustment = -1;
	shift(slope, sz, adjustment);
	// adjustment = pGetAlignment(properties, SLOPE_PEAK, sz, onset);
	pShift(properties, SLOPE_PEAK, sz, adjustment);
	// adjustment = getAlignment(fPeakCount, sz, onset);
	shift(fPeakCount, sz, adjustment);
	// adjustment = pGetAlignment(properties, F_PEAK_PEAK, sz, onset);
	pShift(properties, F_PEAK_PEAK, sz, adjustment);
	// adjustment = getAlignment(fSlopePeakCount, sz, onset);
	adjustment = -4;
	shift(fSlopePeakCount, sz, adjustment);
	// adjustment = pGetAlignment(properties, F_S_PEAK_PEAK, sz, onset);
	pShift(properties, F_S_PEAK_PEAK, sz, adjustment);

	// accumulate the data
	if (st > 0) {	// undo the overlap
		end -= 50;
		i = 50;
	} else
		i = 0;
	for (int index = end * cols; i < sz; i++, end++) {
		rowData[index++] = log(env[i]) - 3.5;
		rowData[index++] = properties[i] & AMP_PEAK? 1: 0;
		rowData[index++] = slope[i];
		rowData[index++] = properties[i] & SLOPE_PEAK? 1: 0;
		rowData[index++] = fPeakCount[i];
		rowData[index++] = properties[i] & F_PEAK_PEAK? 1: 0;
		rowData[index++] = fSlopePeakCount[i];
		rowData[index++] = properties[i] & F_S_PEAK_PEAK? 1: 0;
		index++;	// skip onset marker (already set)
	}
	delete s;
	delete[] FDdata;
	delete frame;
} // processChunk()

void getWeights(double* weights, int cols) {
	for (int i = 0; i < cols; i++) {
		ostrstream buff;
		buff << 'W' << i << ends;
		weights[i] = parameters->getDouble(buff.str());
		buff.freeze(0);		// unlock memory locked by str()
	}
} // getWeights

void setWeights(double* best, double* weights, int cols, int count, int maxc) {
	if (count < 0) {					// default initialisation
		for (int i = 0; i < cols; i++)
			weights[i] = 10;
		weights[cols-1] = 25;
	} else if (count > 0) {				// modify best
		memcpy(weights, best, cols * sizeof(double));
		for (int i = 0; i < 11 - count * 10 / maxc; i++)
			weights[int(rand()/(RAND_MAX+1.0)*cols)] +=
				double(rand() - RAND_MAX/2) / RAND_MAX *
				double(maxc - count + 10) / double(maxc / 10);
	} else								// initialise best
		memcpy(best, weights, cols * sizeof(double));
} // setWeights()

void OnsetLearner :: predictOnsets(int* onsets, int& count, double* weights) {
	// predict the actual onsets
	double* p = rowData;
	int i, score[rows];
	memset(score, 0, rows * sizeof(int));
	for (i = 0; i < rows; i++, p += cols) {
		score[i] += int(threshold((p[0] * weights[0] / 3), 0, weights[0]));
		if (p[1]) {
			if (i > 0) score[i-1] += (int)weights[1] / 2;
			score[i] += (int)weights[1];
			if (i < rows-1) score[i+1] += (int)weights[1] / 2;
		}
		score[i] += int(threshold((p[2] / 10), 0, weights[2]));
		if (p[3]) {
			if (i > 0) score[i-1] += (int)weights[3] / 2;
			score[i] += (int)weights[3];
			if (i < rows-1) score[i+1] += (int)weights[3] / 2;
		}
		// score[i] += int(threshold(p[4], 3, 3 + weights[4])) - 3;
		score[i] += int(threshold(p[4] / 300, 3, 3 + weights[4])) - 3;
		if (p[5]) {
			if (i > 0) score[i-1] += (int)weights[5] / 2;
			score[i] += (int)weights[5];
			if (i < rows-1) score[i+1] += (int)weights[5] / 2;
		}
		// score[i] += int(threshold(p[6], 3, 3 + weights[6])) - 3;
		score[i] += int(threshold(p[6] / 100, 3, 3 + weights[6])) - 3;
		if (p[7]) {
			if (i > 0) score[i-1] += (int)weights[7] / 2;
			score[i] += (int)weights[7];
			if (i < rows-1) score[i+1] += (int)weights[7] / 2;
		}
	}
	findPeaks(score, rows, 3, (int)weights[8], onsets, count);
} // predictOnsets()

void OnsetLearner :: printLearningData() {
	if (!strcmp(parameters->getString("showLearningData", "off"), "on")) {
		cout << "@relation onset\n\n"
				"@attribute amp real\n"
				"@attribute peakAmp {y,n}\n"
				"@attribute slope real\n"
				"@attribute peakSlope {y,n}\n"
				"@attribute fPeakCount real\n"
				"@attribute fPeakPeak {y,n}\n"
				"@attribute fSlopePeakCount real\n"
				"@attribute fSlopePeakPeak {y,n}\n"
				"@attribute isOnset {y,n}\n\n"
				"@data\n";
		cout << setprecision(1);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++)
				cout << setw(6) << rowData[i*cols + j] << (j==cols-1?"\n":", ");
		}
	}
} // printLearningData()

OnsetLearner :: OnsetLearner(const char* name) {
	// get parameters
	{ sample getLength(name, 0, 0.01); } // for side effect (setting fileLength)
	audioLength = parameters->getDouble("fileLength", 0);
	audioLength = parameters->getDouble("learnLength", audioLength);
	chunkSize = parameters->getInt("chunkLength", 60);
	maxOnsetRate = 50;	// 50 onsets per second is probably enough
	fileName = name;
	deltaTime = parameters->getDouble("hopSize", 0.01);
	rows = int(rint(audioLength / deltaTime));
	cols = 9;
	rowData = new double[rows * cols];
	memset(rowData, 0, rows * cols * sizeof(int));
	end = 0;

	// load MIDI file with correct onset times
	realCount = 0;
	midiName = parameters->getString("midiFile");
	if (midiName) {
		eventList* real = new eventList(midiName);
		realOnsets = new double[real->count(NOTE_ON)];
		real->align(parameters->getDouble("firstNote",-1));
		for (real = real->nextNote(); real->hasMore(); real = real->nextNote()){
			if (real->ev->onset > audioLength)
				break;
			if (real->ev->volume > 20)	// ignore really quiet notes
				realOnsets[realCount++]  = real->ev->onset;
		}
		delete real;
		int i;
		for (int t = i = 0; (i < realCount) && (t < rows); i++) {
			t = int(rint(realOnsets[i] / deltaTime));
			if (t < rows)
				rowData[t * cols + cols - 1] = 1;
		}
	}

	// process audio data, chunkwise since it might not fit in memory
	for (double start = 0; start < audioLength; start += chunkSize) {
		if (audioLength - start < chunkSize + 1)
			processChunk(start, audioLength - start);
		else
			processChunk(start, chunkSize+1);
		cerr << "Processing Chunk at t = " << start << endl;
	}

	// predict onsets and evaluate / learn
	predictedCount = int(audioLength * maxOnsetRate);
	int tmp[predictedCount];
	predictedOnsets = new double[predictedCount];
	printLearningData();
	double weights[cols];
	double bestScore;
	double best[cols];
	char* weightFile = parameters->getString("weightFile");
	int generations = parameters->getInt("generations", 500);
	ifstream* in = NULL;
	if (weightFile)
		in = new ifstream(weightFile);
	int sign = 1;
	do {
		if (weightFile) {
			char buff[256];
			in->getline(buff, 256);
			char* start = buff;
			int c;
			for (c = 0; c < cols; c++) {
				char* stop = strchr(start, ' ');
				if (stop != NULL) {
					*stop = 0;
					parameters->add(start);
					start = stop+1;
				} else {
					parameters->add(start);
					break;
				}
			}
			if (c >= cols - 1)
				getWeights(weights, cols);
			else
				break;
		} else
			setWeights(best, weights, cols, -1, generations);
		bestScore = 0;
		for (int j = 0; j < generations; j++) {
			setWeights(best, weights, cols, j, generations);
			predictedCount = int(audioLength * maxOnsetRate);
			predictOnsets(tmp, predictedCount, weights);
			for (int i = 0; i < predictedCount; i++)
				predictedOnsets[i] = tmp[i] * deltaTime;
			double score = evaluate();
			cout << setprecision(1) << "Weights:";
			for (int i = 0; i < cols; i++)
				cout << setprecision(1) << setw(8) << weights[i];
			cout << "\nScore " << j << " : " << setw(6) << score << endl;
			if (score > bestScore) {
				memcpy(best, weights, cols * sizeof(double));
				bestScore = score;
			}
		}
	} while (weightFile && !in->eof());
	delete in;
} // OnsetLearner() constructor

double OnsetLearner :: evaluate() {
	int next = 0;
	bool matched = false;
	int doubleMatches = 0;
	double sumAllErrors = 0;
	double sumSmallErrors = 0;
	int countLargeErrors = 0;
	int countFalsePositives = 0;
	const double small = 0.070;
	cout << setprecision(3);
	if (predictedCount)
		for (int i = 0; i < realCount; i++) {
			double t = realOnsets[i];
			while ((next < predictedCount) && (predictedOnsets[next] < t)) {
				if (!matched)
					countFalsePositives++;
				if (!strcmp(parameters->getString("showOnsetMatch","off"),"on"))
					cout << endl << setw(8) << predictedOnsets[next];
				next++;
				matched = false;
			}
			if ((next < predictedCount) && ((next == 0) ||
								(t - predictedOnsets[next-1] >=
								 predictedOnsets[next] - t))) {
				if (!matched)
					countFalsePositives++;
				if (!strcmp(parameters->getString("showOnsetMatch","off"),"on"))
					cout << endl << setw(8) << predictedOnsets[next];
				next++;
				matched = false;
			}
			if (matched) {
				doubleMatches++;
				if (!strcmp(parameters->getString("showOnsetMatch","off"),"on"))
					cout << endl << "        ";
			}
			matched = true;
			double err = fabs(predictedOnsets[next-1] - t);
			if (!strcmp(parameters->getString("showOnsetMatch","off"),"on"))
				cout << setw(8) << t << setw(8) << err;
			sumAllErrors += err;
			if (err < small)
				sumSmallErrors += err;
			else
				countLargeErrors++;
		}
	double score = (predictedCount - countFalsePositives) * 100 /
					double(predictedCount + countLargeErrors);
	cout << "\n\nReal onsets:         " << realCount
		 << "\nDouble matches:        " << doubleMatches
		 << "\nDetected onsets:       " << predictedCount
		 << "\nLarge error count:     " << countLargeErrors
		 << "\nFalse positive count:  " << countFalsePositives
		 << "\nTotal error rate:      " << (sumAllErrors / realCount)
		 << "\nSmall (<" << small
		 << ") err rt: " << sumSmallErrors / (realCount - countLargeErrors)
		 << "\nScore:                 " << score
		 << endl;
	return score;
} // evaluate()

void onsetLearn(const char* name) { OnsetLearner ol(name); }
