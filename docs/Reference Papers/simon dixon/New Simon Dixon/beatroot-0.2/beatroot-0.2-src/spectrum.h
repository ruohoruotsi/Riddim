//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: spectrum.h
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

class eventList;
class sample;
class tracker;

class spectrum {
		sample* source;
		int sourceStart;
		int size;
		int support;
		int slide;
		double rmsAmplitude;
		double smoothedAmplitude;
		double smoothingFactor;
		double piSlice;
		double loThreshold;
		double* data;
		double* data2;
		double* relativePhaseChange;
		double* windowFunction;
		void updateFrame();

	public:
		spectrum(sample* s, int wsize, int wsupport, int wslide);
		bool nextFrame(int start = -1);
		void toLogFreq(double* data, double* amp, int timeNow,
					   int timeCount, double fMin, int freqCount);
		void makeTracks(int timeNow, tracker* tracks);
		double windowCorrection();
		double getOverlap() { return double(size)/double(slide); }
		double logFreqComponent(double freq);
		double amplitudeComponent(int index);
		double powerComponent(int index);
		double getRMSAmplitude() { return rmsAmplitude; }
		double getSmoothedAmplitude() { return smoothedAmplitude; }
};

class spectrogram {
		double fMin, tInc;
		int fCount, tCount;
		double* data;
		double* amplitude;
		double loThreshold, hiThreshold;
		spectrum* frame;
		tracker* tracks;

	public:
		spectrogram(sample* s);
		double power(int pitch, int time);
		eventList* getEvents();
		eventList* getTracks();
		void queryData();
		void psOut(char* filename);
        int getSizeF() { return fCount; }
        int getSizeT() { return tCount; }
        double getScaleT() { return tInc; }
        double getStartF() { return fMin; }
        double* getData() { return data; }
		double getLowThreshold() { return loThreshold; }
		double getHighThreshold() { return hiThreshold; }
		double getOverlap() { return frame->getOverlap(); }
};

class tracker {
		eventList* head;
		eventList* aliveHead;
		double hiThreshold;
		double tInc;
	public:
		tracker(double windowSlide);
		void print();
		void add(double freq, double power, double time);
		eventList* finalEvents();
};

class psFile {
		char* name;
		int length,	width,			// of one data point (width is for freq)
			pageCount, lineCount, pageSize,
			tCount,					// no of time blocks (frames)
			fCount;					// no of frequency blocks
		double fMin,				// min frequency plotted
			tInc,					// time between successive frames
			min, max;				// threshold values for white/black
		ofstream f;
		void psHeader();
		void psTrailer();

	public:
		psFile(char* fileName, int fCount, int tCount, double fMin,double tInc);
		void psEnd();
		void psData(double* data, int time);
};
