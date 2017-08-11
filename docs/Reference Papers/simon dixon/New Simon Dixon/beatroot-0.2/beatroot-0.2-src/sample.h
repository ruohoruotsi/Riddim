//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sample.h
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

#include "local.h"
#include "sfheader.h"

class eventList;	// forward definition
class sample;		// forward definition
class systemRT;		// forward definition


void playSongList(char** songList);
void audioBeatTrack(const char* fileName);
void midiBeatTrack(const char* fileName);
// void onsetLearn(const char* fileName);
void showDistributions(char* fileName);


class sample {
public:
	audioSample* data;
private:
	sfheader info;

public:
	// constructors/destructor
	sample();
	sample(const sample& s);
	sample(int length = 0, int rate = 0, int channels = 1);
	sample(const float* fData, int len, int rt, int ch = 1);
	sample(const char* filename, int start=-1, int stop=-1, bool secs=false);
	~sample();

	// basic manipulation
	void clear();
	void append(const sample& t);
	void prepend(const sample& t);
	sample* copy(int start, int end=-1);
	void paste(const sample& t, int start=0, bool replace = true);
	void sum(const sample& t, int start=0);
	int diff(const sample& t) const;
	void fade(int seconds = 3, int offset = -1);
	sample* appendChannels(const sample& t, bool replace = false);
	void mix(double bias = 0.5);
	void remix();
	sample* makeMono();
	void makeName(const char* prefix, const char* suffix, char* dest) const;
	double rmsVolume(int start = 0, int end = -1);
	double rmsEstimate(int start, int end, int skip = 1);

	// advanced operations
	sample* crossfade(const sample& t) const;
	sample* interpolateRate(int newrate) const;
	sample* changeRate(int newrate, int start=0, int ln=-1, int LIMIT=50) const;
	sample* warpRate(int finalrate, int LIMIT=50) const;
	void applyFilter(double* a, double* b, double* c, double* d, int n);
	sample* downSample(int filterFreq, int newRate);

	// beat tracking operations
	eventList* getOnsets(eventList** env=NULL,
						 double start=0, double len=-1.0) const;
	void annotate(int initialTempo,int initialPhase,int endTempo,int endPhase);
	void overlayClickTrack(const eventList* beatList);
	sample* makeClickTrack(const eventList* beatList, int len = -1) const;
	sample* addClickTrack(const eventList* beatList);

	// I/O operations
	void writeFile(char *filename, double start = 0, double len = -1.0);
	void writeSndFile(const char *filename, int offset=0, int size = -1);
	void writeWavFile(const char *filename, int offset=0, int size = -1);
	void print() const;
	float* toFloatArray() const;
	double* toDoubleArray() const;
	void play(double start=0.0, double length=-1.0);
	void iPlay();

	// access/update
	int rate() const { return info.rate; }
	int channels() const { return info.channels; }
	int length() const { return info.length; }
	int audioSize() const { return info.length * info.channels; }
	int bytes() const { return audioSize() * sizeof(audioSample); }
	char* text() { return info.text; }
	int textlength() const { return info.textlength; }
	void setRate(int r) { info.rate = r; }
	void setText(char *t) { info.text = t; }
	void setTextlength(int t) { info.textlength = t; }
};

