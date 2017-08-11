//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sampleBeat.cpp
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
#include "param.h"
#include "sample.h"
#include "util.h"


//   IMPLEMENTATION of CLASS SAMPLE (part)
//   -- onset detection routines for beat tracking

#define METRONOME "/raid/music/audio/percussion/33-metclick.wav"
// #define METRONOME "/raid/user/simon/audio/drum/56-cowbell.wav"

// Creates a comment containing the beat/phase details for SUN audio files
void sample :: annotate(int initialTempo, int initialPhase,
						int finalTempo, int finalPhase) {
	if (textlength() != 24) {
		if (text())
			delete [] text();
		setText(new char[24]);
		setTextlength(24);
	}
	strcpy(text(), "beaTmix");
	sWrite32BE(initialTempo, (unsigned char*)(text()+8));
	sWrite32BE(initialPhase, (unsigned char*)(text()+12));
	sWrite32BE(finalTempo, (unsigned char*)(text()+16));
	sWrite32BE(finalPhase, (unsigned char*)(text()+20));
} // annotate(int,int,int,int)

// General peak picking method for finding local maxima in an int array
void findPeaks(	int* base,			// data array
				int length,			// length of data array
				int peakWidth,		// ignore peaks as close as this
				int threshold,		// ignore peaks below this level
				int* peaks,			// array for output
				int& peakLength) {	// length of output array
	int* maxp = base;
	int* centre = base;
	int* i = base;
	int* end = base + length;
	int maxPeaks = peakLength;

	peakLength = 0;
	while (i<end) {
		while ((i<end) && (i <= centre+peakWidth)) {
			if (*i > *maxp)
				maxp = i;
			i++;
		}
		if (i == end)
			break;
		if (maxp > centre)
			centre = maxp;
		else if (maxp < centre) {
			maxp++;
			i = maxp;
			centre = maxp+peakWidth;
		} else {
			if (parameters->debug("sampleBeat", "showPeaks"))
				cerr << "found max: " << maxp-base << " : " << *maxp << endl;
			if (*maxp > threshold) {
				peaks[peakLength++] = maxp - base;
				if (!assertWarning(peakLength<maxPeaks,"findPeaks(): overflow"))
					peakLength--;
			} else if (parameters->debug("sampleBeat", "showPeaks"))
				cerr << "Below average value of: " << threshold << endl;
			while ((centre < end-1) && (*centre >= *(centre+1)))
				centre++;
			while ((centre < end) && (*centre <= *(centre-peakWidth)))
				centre++;
			i = centre-peakWidth;
			if (i < base)
				i = base;
			maxp = i;
		}
	}
	if (maxp >= centre) {
		if (parameters->debug("sampleBeat", "showPeaks"))
			cerr << "found max: " << maxp-base << " : " << *maxp << endl;
		if (*maxp > threshold)
			peaks[peakLength++] = maxp - base;
		else if (parameters->debug("sampleBeat", "showPeaks"))
			cerr << "Below average value of: " << threshold << endl;
	}
} // findPeaks()

// Returns an amplitude envelope of the signal
void makeAmplitudeEnvelope(audioSample* ptr, int length, int slide,	// input
							int*& env, int& sz, double& average) {	// output
	int overlap = parameters->getInt("overlap", 2);
	const int frameSize = overlap * slide;
	sz = length / slide;
	if (!assertWarning(sz > 3, "Insufficient audio data")) {
		sz = 0;
		average = 0;
		env = NULL;
		return;
	}
	env = new int[sz];
	int last[overlap];
	int i,j;
	average = 0.0;
	last[0] = 0;
	for (i=0; i<slide; i++)
		last[0] += abs(ptr[i] - ptr[i+1]);
	for (i=1; i<overlap; i++)
		last[i] = last[0];
	env[0] = last[0] * overlap / frameSize;
	average = env[0];
	for (j=1; j <= sz-overlap; j++) {
		last[0] = 0;
		for (i=0; i<slide-1; i++)
			last[0] += abs(ptr[i+slide*j] - ptr[i+slide*j+1]);
		env[j] = last[0];
		for (i = overlap - 1; i > 0; i--) {
			env[j] += last[i];
			last[i] = last[i-1];
			}
		env[j] /= frameSize;
		average += env[j];
	}
	for ( ; j < sz; j++) {
		env[j] = last[0];
		for (i = overlap - 1; i > 0; i--) {
			env[j] += last[i];
			last[i] = last[i-1];
		}
		env[j] /= frameSize;
		average += env[j];
	}
	average /= sz;
} // makeAmplitudeEnvelope()

event* percussionEvent(double onset, double beatNumber = UNKNOWN);

// Finds onsets in an audio envelope using Schloss's surfboard method
eventList* surf(int* env,int sz,double average,double deltaTime) {
	eventList* evList = new eventList();
	if (sz <= 3)
		return evList;

	// Perform 4-pt linear regression to find slope
	int slope[sz];
	int sx = 6;						// x=0,1,2,3; arbitrary choice; =spacing
	int sxx = 14;					// x^2=0,1,4,9
	int delta = 4 * sxx - sx * sx;
	int sy = env[0] + env[1] + env[2] + env[3];
	int sxy = env[1] + 2*env[2] + 3*env[3];
	int i,j;
	for (j=0; j<sz-4; j++) {
		slope[j] = (4 * sxy - sx * sy) / delta;
		sy += env[j+4] - env[j];
		sxy += 4 * env[j+4] - sy;
	}
	slope[j] = (4 * sxy - sx * sy) / delta;
	
	// find peaks in slope
	// threshold for peaks is average/thresholdFactor
	int peakLength = sz;
	int peaks[peakLength];
	double thresholdFactor = parameters->getDouble("thresholdFactor", 10.0);
	int wd = parameters->getInt("peakWidth", 5);
	findPeaks(slope, sz-4, wd, int(average/thresholdFactor), peaks, peakLength);

	// convert int array to eventList
	for (j = 0; j < peakLength; j++) {
		event* e = percussionEvent((double)peaks[j] * deltaTime);
		e->salience = threshold(80.0 + (80.0 *
			log10((double)env[peaks[j]] / average)), 10.0, 127.0);
		evList->add(e);
	}
	if (parameters->debug("sampleBeat", "showOnsets")) {
		cerr << "surf(): detected event onsets:" << endl;
		evList->print();
	}
	return evList;
} // surf()

eventList* amplitudePeaks(int* env, int sz, double average, double deltaTime) {
	eventList* evList = new eventList();
	if (sz <= 0)
		return evList;
	int peakLength = sz;
	int peaks[peakLength];
	double thresholdFactor2 = parameters->getDouble("thresholdFactor2", 10.0);
	int width = parameters->getInt("peakWidth", 5);
	findPeaks(env, sz, width, int(average/thresholdFactor2), peaks, peakLength);

	// convert int array to eventList
	for (int j = 0; j < peakLength; j++) {
		event* e = percussionEvent((double)peaks[j] * deltaTime);
		e->salience = threshold(80.0 + (80.0 *
			log10((double)env[peaks[j]] / average)), 10.0, 127.0);
		evList->add(e);
	}
	if (parameters->debug("sampleBeat", "showAmplitudePeaks")) {
		cerr << "amplitudePeaks(): detected peaks in envelope:" << endl;
		evList->print();
	}
	return evList;
} // amplitudePeaks()

eventList* convertEnvelope(int* env, int len, double scale, double deltaTime) {
	eventList* evList = new eventList();
	for (int i = 0; i < len; i++) {
		event* e = new event((double)i * deltaTime);
		e->salience = threshold(80.0 + (80.0 *
			log10((double)env[i] / scale)), 0.0, 127.0);
		evList->add(e);
	}
	return evList;
} // convertEnvelope()

// Finds note onsets in an audio sample
eventList* sample ::
getOnsets(eventList** envelope, double st, double ln) const {
	if (!assertWarning(channels() == 1, "expected mono audio signal"))
		return new eventList();
	int start = (int) rint(st * rate());
	if (!assertWarning((start >= 0) && (start < length()),
						"Ignoring illegal start position in getOnsets()"))
		start = 0;
	int len = (int) rint(ln * rate());
	if ((len <= 0) || (start + len > length()))
		len = length() - start;
	int* env;
	int count;
	double average;
	int slide = int(parameters->getDouble("hopSize", 0.01) * rate());
	makeAmplitudeEnvelope(data+start, len, slide, env, count, average);
	double deltaTime = (double)slide / (double)rate();
	eventList* onsets = surf(env, count, average, deltaTime);
	if (envelope)
		*envelope = convertEnvelope(env, count, average, deltaTime);
	if (env != NULL)
		delete[] env;
	if (char* out = parameters->getString("clickTrackOnly")) {
		// for conference examples
		sample* clickTrack = makeClickTrack(onsets);
		clickTrack->writeFile(out);
		clickTrack->play();
		delete clickTrack;
	}
	return onsets;
} // getOnsets()

// Calculates the (inverse) tempo and phase, measured in numbers of samples
//eventList* sample :: beatTrack(eventList* onsets, double st, double ln) {
//	// eventList* onsets = getOnsets();
//	if (onsets == NULL)
//		onsets = getOnsets();
//	agentList* trackers = onsets->beatInduction();
//	trackers->beatTrack(onsets);
//	agent* tracker = trackers->bestAgent();
//	assert(tracker != NULL, "Beat tracking failed");
//	if (!strcmp(parameters->getString("showTracking","on"),"on"))
//		tracker->showTracking(onsets);
//	tracker->fillBeats();
//	// tracker->events->printStatistics();	// does nothing since pitch == 0
//	if (strcmp(parameters->getString("evaluate","off"), "on") == 0) {
//		char outputName[250];
//		makeName("exp/audio-bt/", ".beat", outputName);
//		eventList* beatList = new eventList(outputName);
//		beatList->evaluate(tracker->events);
//	}
//	eventList* ev = new eventList(tracker->events);
//	delete trackers;
//	return ev;
//} // beatTrack()

void audioBeatTrack(const char *fileName) {
	sample* outputSample;
	sample* audioData = new sample(fileName);
	if (!assertWarning(audioData->data != NULL, "Unable to create sample")) {
		delete audioData;
		return;
	}
	eventList* onsets = audioData->getOnsets();
	eventList* beats = onsets->beatTrack();
	if (!strcmp(parameters->getString("stereo","on"),"on")) { // separate track
		outputSample = audioData->addClickTrack(beats);
	} else {	// add click track to current track(s)
		audioData->overlayClickTrack(beats);
		outputSample = audioData;
	}
	if (char* out = parameters->getString("audioOut")) {
		outputSample->writeFile(out);
	}
	if (char* out = parameters->getString("beatsOut")) {
		beats->writeTMF(out);
	}
	if (!strcmp(parameters->getString("play","on"),"on"))
		outputSample->play();
	if (outputSample != audioData)
		delete outputSample;
	delete audioData;
	delete onsets;
	delete beats;
} // audioBeatTrack()

sample* makeClick(int rate) {
	sample* click = new sample(parameters->getString("metronome",METRONOME), 0);
	if (!assertWarning(click->data != NULL, "Unable to create sample")) {
		delete click;
		return NULL;
	}
	if (click->rate() != rate) {
		sample* click2 = click->changeRate(rate);
		delete click;
		return click2;
	}
	return click;
} // makeClick()

// Example for conference presentation - what does an eventList sound like?
sample* sample :: makeClickTrack(const eventList* list, int len) const {
	const int RATE = rate();
	sample* click = makeClick(RATE);
	if (!assertWarning(click != NULL, "Unable to create sample"))
		return NULL;
	if ((len < 0) && (list != NULL) && (list->prev->ev != NULL))
		len = (int) ceil(list->prev->ev->onset);// whole file or first len secs?
	sample* clickTrack = new sample(len * RATE + click->length(), RATE);
	if (!assertWarning(clickTrack->data != NULL, "Unable to create sample")) {
		delete clickTrack;
		if (click != NULL)
			delete click;
		return NULL;
	}
	eventList* e = (eventList*) list;
	if (e != NULL)
		for (e = e->nextNote(); e->ev != NULL; e = e->nextNote())
			clickTrack->paste(*click, (int)rint(e->ev->onset * RATE), false);
	delete click;
	return clickTrack;
} // makeClickTrack()

void sample :: makeName(const char* prefix,const char* suffix,char* dest) const{
	makeFileName(info.filename, prefix, suffix, dest);
}

// Adds clicks to the track at the locations given in list, for aural testing
void sample :: overlayClickTrack(const eventList* list) {
	if (list != NULL) {
		sample* click = makeClick(rate());
		eventList* e = (eventList*) list;
		for (e = e->nextNote(); e->hasMore(); e = e->nextNote())
			paste(*click, (int) rint(e->ev->onset * rate()), false);
		delete click;
	}
} // overlayClickTrack()

sample* sample :: addClickTrack(const eventList* list) {
	sample* click = makeClickTrack(list);
	sample* output = appendChannels(*click, channels() == 2);
	delete click;
	return output;
} // addClickTrack()

