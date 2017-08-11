//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sample.cpp
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
#include "param.h"
#include "playq.h"
#include "sample.h"
#include "util.h"
#include <cstdio>


// Basic operations for CLASS SAMPLE

// Constructors and destructor

sample ::
sample() : data(0) {
	throw Exception("sample(): default constructor not valid");
} // sample default constructor

sample ::
sample(const sample& s) : data(0), info(s.info) {
	data = new audioSample[audioSize()];
	memcpy(data, s.data, bytes());
	if (parameters->debug("sample", "basic")) { cerr << "Created: "; print(); }
} // sample copy constructor

sample ::
sample(int len, int rt, int ch): data(0), info(len, rt, ch) {
	data = new audioSample[audioSize()];
	clear();
	if (parameters->debug("sample", "basic")) { cerr << "Created: "; print(); }
} // sample constructor (blank)

sample ::
sample(const float* fData, int ln, int rt, int ch): data(0), info(ln,rt,ch) {
	data = new audioSample[audioSize()];
	for (int i = audioSize() - 1; i >= 0; i--)
		data[i] = audioLimit((int) rint(32768.0 * fData[i]));
	if (parameters->debug("sample", "basic")) { cerr << "Created: "; print(); }
} // sample constructor (float array)

// Constructs a sample object from a sound file (.wav/.snd PCM files only).
// Reads stereo files into mono by averaging channels (unless the channels
//   option is used).
// start & stop are counted in samples (default) unless secsFlag == true.
// Use stop == -1 (default) for reading to EOF.
sample ::
sample(const char *filename, int start, int stop, bool secsFlag): data(0) {
	ifstream f(filename);
	if (!f.is_open())
		throw Exception("sample constructor: could not open file");
	info = sfheader(f, filename);
	char buf[40];
	sprintf(buf, "fileLength=%3.1lf\n", double(length()) / double(rate()));
	parameters->add(buf);
	if (secsFlag) {
		start *= rate();
		stop *= rate();
	}
	if (start < 0) { // uses start&length unless audioStart&audioLength override
        double st = parameters->getDouble("start", 0);
		start = (int) rint(parameters->getDouble("audioStart", st) * rate());
		double len = parameters->getDouble("length", -1.0);
		int ln = (int)rint(parameters->getDouble("audioLength", len) * rate());
		if (ln >= 0)
			stop = start + ln;
		else
			stop = -1;
	}
	if (stop < 0)
		stop = length();
	if (!assertWarning((start >= 0) && (start < length()),
			"sample(): illegal start parameter"))
		start = 0;
	if (!assertWarning((start <= stop) && (stop <= length()),
			"sample(): illegal length parameter"))
		stop = length();
	info.length = stop - start;	// number of sample tuples
	f.seekg(start * channels() * sizeof(audioSample), ios::cur);
	int count = length();
	char* channelChoice = parameters->getString("channels", "add");
	if (!strcmp(channelChoice, "both"))
		count *= channels();
	data = new audioSample[count];
	if (!strcmp(channelChoice, "both") || (channels() == 1)) {	// MONO or Keep
		if (info.format == WAV_LINEAR_PCM)
			for (int i = 0; i < count; i++)
				data[i] = read16LE(f);
		else
			for (int i = 0; i < count; i++)
				data[i] = read16BE(f);
	} else if (channels() == 2) {				// STEREO Merge or ChannelSelect
		if (info.format == WAV_LINEAR_PCM) {
			if (!strcmp(channelChoice, "subtract"))
				for (int i = 0; i < count; i++)
					data[i] = (read16LE(f) - read16LE(f)) / 2;
			else if (!strcmp(channelChoice, "add"))
				for (int i = 0; i < count; i++)
					data[i] = (read16LE(f) + read16LE(f)) / 2;
			else {
				if (!strcmp(channelChoice, "right"))
					read16LE(f);	// skip first sample (left channel)
				for (int i = 0; i < count-1; i++) {
					data[i] = read16LE(f);
					read16LE(f);	// skip every 2nd sample
				}
				if (count > 0)
					data[count-1] = read16LE(f);	// avoid reading past EOF
			}
		} else {
			if (!strcmp(channelChoice, "subtract"))
				for (int i = 0; i < count; i++)
					data[i] = (read16BE(f) - read16BE(f)) / 2;
			else if (!strcmp(channelChoice, "add"))
				for (int i = 0; i < count; i++)
					data[i] = (read16BE(f) + read16BE(f)) / 2;
			else {
				if (!strcmp(channelChoice, "right"))
					read16BE(f);	// skip first sample (left channel)
				for (int i = 0; i < count; i++) {
					data[i] = read16BE(f);
					read16BE(f);	// skip every 2nd sample
				}
			}
		}
		info.channels = 1;
	} else {									// MULTI_CHANNEL Merge
		for (int i = 0; i < count; i++) {
			int val = 0;
			for (int j = 0; j < channels(); j++)
				val += (info.format == WAV_LINEAR_PCM)?read16LE(f):read16BE(f);
			data[i] = val / channels();
		}
		info.channels = 1;
	}
	if (parameters->debug("sample", "basic")) { cerr << "Created::"; print(); }
} // sample constructor

sample :: ~sample() {
	if (parameters->debug("sample", "basic")) {
		cerr << "Destroying: ";
		print();
	}
	if (playq->notUsing(this) && (data != NULL))
		delete[] data;
} // sample destructor


// Simple operations

void sample :: clear() {
	memset(data, 0, bytes());
} // clear()

void sample :: append(const sample& s) {
	if (!assertWarning(rate() == s.rate(), "append failed: different rates") ||
		!assertWarning(channels() == s.channels(),
			"append failed: different channel counts"))
		return;
	audioSample* snd = new audioSample[audioSize() + s.audioSize()];
	memcpy(snd, data, bytes());
	memcpy(snd + audioSize(), s.data, s.bytes());
	delete[] data;
	data = snd;
	info.length += s.length();
} // append()

void sample :: prepend(const sample& s) {
	if (!assertWarning(rate() == s.rate(), "prepend failed: different rates") ||
		!assertWarning(channels() == s.channels(),
			"prepend failed: different channel counts"))
		return;
	audioSample *snd = new audioSample[audioSize() + s.audioSize()];
	memcpy(snd, s.data, s.bytes());
	memcpy(snd + s.audioSize(), data, bytes());
	delete[] data;
	data = snd;
	info.length += s.length();
} // prepend()

sample* sample :: copy(int start, int end) {
	if (end < 0)
		end = length();
	if (!assertWarning((start>=0)&&(start<=end)&&(end<=length()), 
			"copy: bad parameters"))
		return NULL;
	sample* r = new sample(end-start, rate(), channels());
	if (r->data == NULL) {
		delete r;
		return NULL;
	}
	memcpy(r->data,data+start*channels(),r->bytes());
	return r;
} // copy()

void sample :: paste(const sample& clip, int start, bool replaceFlag) {
	if (!assertWarning(rate() == clip.rate(),"paste failed: different rates") ||
		!assertWarning(channels() == clip.channels(),
			"paste failed: different channel counts"))
		return;
	int limit = clip.length();
	if (start + limit > length())
		limit = length() - start;
	if (replaceFlag)
		memcpy(data+start*channels(), clip.data,
				limit * channels() * sizeof(audioSample));
	else
		for (int i = 0; i < limit * channels(); i++, start++)
			data[start] = audioLimit(clip.data[i] + data[start]);
} // paste()

void sample :: sum(const sample& s, int start) {
	paste(s, start, false);
} // sum()

int sample :: diff(const sample& t) const {
	if (!assertWarning(rate() == t.rate(),"diff: sample rates") ||
		!assertWarning(length() == t.length(),"diff: different lengths") ||
		!assertWarning(channels() == t.channels(), "diff: channel counts"))
		return 1;
	int diffs = 0;
	for (int i=0; (i<audioSize()) && (diffs<10); i++)
		if (data[i] != t.data[i]) {
			if (parameters->debug("sample", "basic"))
				cerr << "Data differs at " << i << " of " << audioSize() <<endl;
			diffs++;
		}
	if (parameters->debug("sample", "basic") && !diffs)
		cerr << "No diffs" << endl;
	return diffs;
} // diffs()

sample* sample :: appendChannels(const sample& toAdd, bool replace) {
	if (!assertWarning(rate()==toAdd.rate(),"appendChannels(): sampling rates"))
		return this;
	int keepChannels = replace? channels() - toAdd.channels(): channels();
	int totalChannels = keepChannels + toAdd.channels();
	if (!assertWarning(keepChannels > 0, "will not replace all channels"))
		return this;
	int minLength, maxLength;
	if (length() < toAdd.length()) {
		minLength = length();
		maxLength = toAdd.length();
	} else {
		minLength = toAdd.length();
		maxLength = length();
	}
	sample* outSample = this;
	if (!replace) {
		outSample = new sample(maxLength, rate(), totalChannels);
		if (outSample->data == NULL) {
			delete outSample;
			return NULL;
		}
	}
	audioSample* inPtr = data;
	audioSample* addPtr = toAdd.data;
	audioSample* outPtr = outSample->data;
	int i,j;
	for (i = 0; i < minLength; i++) {
		if (replace)
			for (j = 0; j < keepChannels; j++)
				outPtr++;
		else
			for (j = 0; j < keepChannels; j++)
				*outPtr++ = *inPtr++;
		for ( ; j < totalChannels; j++)
			*outPtr++ = *addPtr++;
	}
	while (i < length()) {		// if this is longer
		if (replace)
			for (j = 0; j < keepChannels; j++)
				outPtr++;
		else
			for (j = 0; j < keepChannels; j++)
				*outPtr++ = *inPtr++;
		for ( ; j < totalChannels; j++)
			*outPtr++ = 0;
		i++;
	}
	if (!replace)
		while (i < toAdd.length()) {	// if t is longer
			for (j = 0; j < keepChannels; j++)
				*outPtr++ = 0;
			for ( ; j < totalChannels; j++)
				*outPtr++ = *addPtr++;
			i++;
		}
	return outSample;
} // appendChannels()

// Create a 2-channel mono file from a 2-channel stereo file
void sample :: mix(double bias) {
	if (!assertWarning(channels() == 2, "mix: Not a stereo sample"))
		return;
	int i, temp, len = length();
	audioSample* ptr = data;
	for (i = 0; i < len; i++) {
		temp = int(double(*ptr) * bias + double(*(ptr+1)) * (1 - bias));
		if (abs(temp) > maxAudioSample)
			break;
		*ptr++ = temp;
		*ptr++ = temp;
	}
	if (i < len) {	// overflow occurred
		assertWarning(FALSE, "Halving amplitude to avoid overflow in mix()");
		audioSample* ptr2 = data;
		for (int j = 0; j < i; j++)
			*ptr2++ /= 2;
		for ( ; i < len; i++) {
			temp = int((double(*ptr) * bias + double(*(ptr+1)) * (1 - bias))/2);
			*ptr++ = temp;
			*ptr++ = temp;
		}
	}
} // mix()

// Create a 2-channel stereo file from a 2-channel stereo file
void sample :: remix() {
	if (!assertWarning(channels() == 2, "mix: Not a stereo sample"))
		return;
	int i, tempLeft, tempRight, max, len = length();
	double ilol = parameters->getDouble("ilol", 0.5);
	double ilor = parameters->getDouble("ilor", 0.5); 
	double irol = parameters->getDouble("irol", 0.5); 
	double iror = parameters->getDouble("iror", 0.5);
	audioSample* ptr = data;
	for (i = 0; i < len; i++) {
		double il = *ptr;
		double ir = *(ptr+1);
		tempLeft = int(il * ilol + ir * irol);
		tempRight = int(il * ilor + ir * iror);
		if (abs(tempLeft) > max)
			max = abs(tempLeft);
		if (abs(tempRight) > max)
			max = abs(tempLeft);
		*ptr++ = tempLeft;
		*ptr++ = tempRight;
	}
	if (max > maxAudioSample)
		cerr << "Warning: overflow (" << max << ")\n";
} // remix()

sample* sample :: makeMono() {
	if (!assertWarning(channels() == 2, "makeMono: not a stereo sample"))
		return NULL;
	sample* s = new sample(length(), rate(), 1);
	if (s->data == NULL) {
		delete s;
		return NULL;
	}
	audioSample* p1 = data;
	audioSample* p = s->data;
	int len = length();
	for (int i = 0; i < len; i++)
		*p++ = (*p1++ + *p1++) / 2;
	return s;
} // makeMono()

double sample :: rmsVolume(int start, int end) {
	if (end == -1)
		end = length();
	return rmsEstimate(start, end);
} // rmsVolume()

double sample :: rmsEstimate(int start, int end, int skip) {
	if (!assertWarning((start < end) && (skip > 0),
				"Illegal parameters in rmsEstimate()"))
		return 0;
	double sum = 0.0;
	start *= channels();
	end *= channels();
	for (int i = start; i < end; i += skip)
		sum += (double)(data[i]) * (double)(data[i]);
	return sqrt(sum / (double)((end - start + skip - 1) / skip));
} // rmsEstimate()

// if (len < 0), fades the beginning -len seconds of the signal (fade in)
//   otherwise (len > 0), fades the final len seconds of the signal (fade out)
void sample :: fade(int len, int offset) {
	int p, dir;
	int ch = channels();
	int sz = abs(len) * rate();
	// double factor = 0;
	double factor = 1.0 / 128.0;
	// double change = 1.0 / double(sz);
	double change = pow(2.0, 7.0 / double(sz));
	if (len < 0) {
		if (offset < 0)
			p = 0;
		else
			p = offset;
		dir = 1;
		if (!assertWarning(p + sz <= audioSize(), "fade.in(): bad parameters"))
			return;
	} else if (len > 0) {
		if (offset < 0)
			p = audioSize() - 1;
		else
			p = offset - 1;
		dir = -1;
		if (!assertWarning((p - sz >= 0) && (p < audioSize()),
				"fade.out(): bad parameters"))
			return;
	}
	for (int i = 0; i < sz; i++) {
		for (int j = 0; j < ch; j++) {
			data[p] = int(rint(factor * double(data[p])));
			p += dir;
		}
		// factor += change;
		factor *= change;
	}

} // fade()
