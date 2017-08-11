//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: spectrum.cpp
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
#include "event.h"
#include "fft.h"
#include "param.h"
#include "sample.h"
#include "spectrum.h"
#include "util.h"


// Implementation of CLASS SPECTRUM

spectrum :: spectrum(sample* s, int wsize, int wsupport, int wslide) {
	smoothingFactor = 1.0 / double(parameters->getInt("smoothingFactor", 20));
	loThreshold = parameters->getDouble("loThreshold", 10.0);
	source = s;
	sourceStart = 0;
	size = wsize;
	support = wsupport;
	slide = wslide;
	data = new double[size+2];
	data2 = new double[size+2];
	relativePhaseChange = new double[size/2];
	piSlice = 2.0 * M_PI / double(size);
	windowFunction = windowChoice(HAMMING, size, support);
	updateFrame();
	smoothedAmplitude = rmsAmplitude;
} // spectrum constructor

bool spectrum :: nextFrame(int start) {
	if (start < 0)		// if start == -1 (default) ==> move on by slide units
		start = sourceStart + slide;
	if (start + size < source->length()) {
		sourceStart = start;
		updateFrame();
		return true;
	} else
		return false;
} // nextFrame()

void spectrum :: updateFrame() {
	asserts(sourceStart + size + 1 <= source->length(),
			"updateFrame(): Unexpected end of source");
	audioSample* sourcePtr = source->data + sourceStart;
	rmsAmplitude = 0;
	for (int i = 0; i < size; i++) {
		data[i] = windowFunction[i] * double(*(sourcePtr++));
	 	// data2[i] = windowFunction[i] * double(*sourcePtr);	// for TPV
		rmsAmplitude += data[i] * data[i];
	}
	rmsAmplitude = sqrt(rmsAmplitude);
	smoothedAmplitude *= 1.0 - smoothingFactor;
	smoothedAmplitude += smoothingFactor * rmsAmplitude;
	fft(data, size);
	// fft(data2, size);	// for TPV
	// for (int i = 0; i < size; i += 2) {
	// 	relativePhaseChange[i/2] = fmod(atan2(data2[i+1], data2[i]) -
	// 		atan2(data[i+1], data[i]) - i/2 * piSlice + 5*M_PI, 2*M_PI) - M_PI;
	// }
} // updateFrame()

double spectrum :: powerComponent(int index) {
	return (data[2*index]*data[2*index] + data[2*index+1]*data[2*index+1]);
}

double spectrum :: amplitudeComponent(int index) {
	return sqrt(powerComponent(index));
}

double spectrum :: logFreqComponent(double freq) {
	double centreBin = freq / double(source->rate()) * double(size);
	double width = centreBin * (pow(2.0, 1.0/36.0) - 1.0);	// +-1/3 semitone
	int bottom = int(floor(centreBin - width));
	int top = int(ceil(centreBin + width));
	// printf("Freq: %5.1lfHz   Width: %4.2lf bins  (%3d,%3d)  %5.3lf %5.3lf\n",
	// 	freq, width, bottom, top,
	// 	1.0 - fmod(centreBin-width, 1), fmod(centreBin+width, 1));
	double maxPower = 
		(1.0 - fmod(centreBin - width, 1)) * powerComponent(bottom) +
		fmod(centreBin - width, 1) * powerComponent(bottom + 1);
	double newPower = 
		(1.0 - fmod(centreBin + width, 1)) * powerComponent(top - 1) +
		fmod(centreBin + width, 1) * powerComponent(top);
	// if (newPower > maxPower)
		maxPower += newPower;
	for (int i = bottom+1; i < top; i++) {
		newPower = powerComponent(i);
		// if (newPower > maxPower)
			maxPower += newPower;
	}
	maxPower /= width;
	if (parameters->debug("spectrum", "showFFTComponents"))
		cout << "logFreq: " << setw(6) << setprecision(1) << freq
			 << "Hz  Bin " << setw(3) << bottom << " to " << setw(3) << top
			 << " (" << setw(6) << centreBin << " +- " << setw(6) << width
			 << "): " << setw(8) << setprecision(2) << maxPower << endl;
	return (maxPower);
} // logFreqComponent()

void spectrum :: toLogFreq(double* outData, double* outAmplitude, int timeNow,
						   int timeCount, double freq, int freqCount) {
	for (int i=0; i < freqCount; i++) {
		outData[i*timeCount+timeNow] = logFreqComponent(freq)    ;//       /smoothedAmplitude;
		freq *= pow(2.0, 1.0/12.0);
	}
	outAmplitude[timeNow] = smoothedAmplitude;
} // toLogFreq()

double spectrum :: windowCorrection() {
	return 0;		// double(size) / double(source->rate());
} // windowCorrection()

void spectrum :: makeTracks(int timeNow, tracker* tracks) {
	double threshold = loThreshold;
	if (rmsAmplitude > threshold)
		threshold = rmsAmplitude;
	threshold *= threshold;
	for (int i=1; i<size/2; i++) {
		if ((relativePhaseChange[i-1] >= 0) && (relativePhaseChange[i] < 0)) {
			double freq = i + 1 - fabs(relativePhaseChange[i]) / 
				(fabs(relativePhaseChange[i-1]) + fabs(relativePhaseChange[i]));
			double power = 0;
			for (int j=i-1; j>=0; j--)
				if (fabs(relativePhaseChange[j] - (freq-j)*piSlice) < piSlice) {
					if (power < powerComponent(j))
						power = powerComponent(j);
				} else
					break;
			for (int j=i; j<size/2; j++)
				if (fabs(relativePhaseChange[j] - (freq-j)*piSlice) < piSlice) {
					if (power < powerComponent(j))
						power = powerComponent(j);
				} else
					break;
			if (power > threshold)
				tracks->add(freq * source->rate() / size, power,
							double(timeNow*slide)/source->rate());
		}
	}
} // makeTracks()

// Implementation of CLASS SPECTROGRAM

spectrogram :: spectrogram(sample* s) {
	int lowestNote = parameters->getInt("lowestNote", 21);
	fMin = midiFrequency(lowestNote);		// 21 = lowest note on piano
	fCount = parameters->getInt("noteCount", 88);	// 88 = # of notes on piano
	tInc = parameters->getDouble("windowSlide", 0.05);
	loThreshold = parameters->getDouble("loThreshold", 10.0);
	hiThreshold = parameters->getDouble("hiThreshold", 1000.0);
	// loThreshold *= loThreshold;
	// hiThreshold *= hiThreshold;

	// frame size
	// n => smallest window that is power of 2 and contains 1/n second
	// for 10kHz sampling rate, this will be 2048 samples (close to 0.2s)
	double freqSep = parameters->getDouble("freqSep", 5.0);
	int wsize = 1 << int(ceil(log( double(s->rate()) / freqSep ) / log(2)));

	// for separate control of time resolution
	int wsupport = int(rint(wsize * parameters->getDouble("support", 1.0)));

	// frame separation
	int wslide = int(rint(tInc * (double)(s->rate())));

	// total number of frames
	tCount = (s->length() - wsize + wslide - 1) / wslide;

	if (parameters->debug("spectrum", "showSpectrogramParameters"))
		cout << "wsize: " << wsize << "; wslide: " << wslide
			 << "; tCount: " << tCount << "; length: " << s->length() << endl;
	data = new double[fCount * tCount];
	amplitude = new double[tCount];
	frame = new spectrum(s, wsize, wsupport, wslide);
	tracks = new tracker(tInc);
	for (int count = 0; count < tCount; frame->nextFrame()) {
		// frame->makeTracks(count, tracks);	// for TPV
		frame->toLogFreq(data, amplitude, count++, tCount, fMin, fCount);
	}
	// printToMatlab(data, fCount * tCount, "junk");
	char* psFile = parameters->getString("psOut");
	if (psFile != NULL) {
		psOut(psFile);
	}
	if (strcmp(parameters->getString("query","off"), "on") == 0)
		queryData();
} // spectrogram constructor

void spectrogram :: queryData() {	// shows amplitude values
	int pageLength = 60; // + 2-line header = 62 lines (for a2ps)
	double startTime, f;
	int lowNote;
	cout << "Midi no: " << flush;
	if (!(cin >> lowNote)) {
		cin.ignore(1000, '\n');
		cin.ignore();
		return;
	} else if ((lowNote < 21) || (lowNote > 108-pageLength-1))
		return;
	cout << "Time: " << flush;
	if (!(cin >> startTime))
		return;
	for (int start = (int)rint(startTime / tInc); start < tCount; start += 20) {
		cout << setprecision(2) << "Time: " << setw(5) << double(start) * tInc
			 << "\nMidi notes " << lowNote << " to " << lowNote + pageLength - 1
			 << endl;
		for (int n = lowNote; n < lowNote + pageLength; n++) {
			for (int t = start; (t < tCount) && (t < start + 20); t++) {
				double sum = 0;
				double sum1 = 0;
				for (f = midiFrequency(n); f < 4200.0; f += midiFrequency(n)) {
					if (f / midiFrequency(n) > 12.5)	// first 12 harmonics
						break;
					double value = sqrt( data[tCount *
								(midiNumber(f) - midiNumber(fMin)) + int(t)]);
					sum += value;
					sum1 += value / f * midiFrequency(n);
				}
				cout << setprecision(0) << setw(4)
					 << sqrt(data[tCount * (n-midiNumber(fMin)) + t]);
			}
			cout << endl;
		} // note loop
	} // time loop
} // queryData()

double spectrogram :: power(int pitch, int time) {
	return data[pitch * tCount + time];
} // power()

int vol(double data, double thresh) {
	return int(rint(threshold(data, thresh/127.0, thresh) / thresh * 127.0));
} // vol()

bool isMidiHarmonic(int midiPitchDifference) {
	switch (midiPitchDifference) {
		case 12: case 19: case 24: case 28: case 31: case 34:
		case 36: case 38: case 40: case 42: case 43: case 48:
			return true;
		default:
			return false;
	}
} // isMidiHarmonic()

// Uses a simple peak picking routine - checks one point either side in 2 dimns
eventList* spectrogram :: getEvents() {
	eventList* e = new eventList();
	double total[tCount];
	if (tCount < 2) return e;
	for (int t = 0; t < tCount; t++) {
		total[t] = 0;
		for (int pitch = 0; pitch < fCount; pitch++)
			total[t] += power(pitch, t);
	}
	for (int pitch = 0; pitch < fCount; pitch++) {
		for (int t = 0; t < tCount; t++)
			if (// power must be greater than minimum threshold
					(power(pitch, t) > loThreshold) &&
				// power must be at least 1% of total power at this time
					(power(pitch, t) / total[t] > 0.01) &&
				// power must be increasing (for an onset)
					((t == 0) || (power(pitch, t) > power(pitch, t-1))) &&
				// power must be at peak value
					(	// (power(pitch, t) > loThreshold * 10.0) ||
						(t == tCount-1) ||
						(power(pitch, t) > power(pitch, t+1)) )
					&&
				// power must be at a peak in the frequency dimension
					((pitch==0) || (power(pitch-1, t) < power(pitch, t))) &&
					((pitch==fCount-1) || (power(pitch+1, t) < power(pitch, t)))
					) {
				double onset = tInc * double(t) + frame->windowCorrection();
				double tmp, volume = 0;
				while ((t < tCount-1) && (power(pitch, t) < power(pitch, t+1))){
					tmp = power(pitch, t++);
					if (tmp > volume)
						volume = tmp;
				}
				while ((t < tCount-1) && (power(pitch, t) > loThreshold) &&
						(power(pitch, t) / power(pitch, t+1) > 0.9)) {
					tmp = power(pitch, t++);
					if (tmp > volume)
						volume = tmp;
				}
				e->insert(new event(onset,
								tInc * double(t+1) + frame->windowCorrection(),
								pitch+midiNumber(fMin),
								vol(volume, hiThreshold), UNKNOWN));
			}
	}
	return e;
} // getEvents()

eventList* spectrogram :: getTracks() {
	return tracks->finalEvents();
} // getTracks()

eventList* eventList :: filterEvents() {
	eventList* e = this;
	for (e = e->next; e->ev != NULL; e = e->next) {
		e->ev->salience = e->ev->volume;
		for (eventList* near = e->nextEvent(e->ev->onset - 0.055);
				((near->ev!=NULL) && (near->ev->onset < e->ev->onset+0.055));
				near = near->next) {
			if (isMidiHarmonic(near->ev->pitch - e->ev->pitch))
				e->ev->salience += near->ev->volume;
		}
	}
	int instr = parameters->getInt("instrument", 1);	// default is piano
	for (e = e->next; e->ev != NULL; e = e->next) {
		e->ev->voice = instr - 1;	// timidity's cfg files aren't consistent
		for (eventList* near = e->nextEvent(e->ev->onset - 0.055);
				((near->ev!=NULL) && (near->ev->onset < e->ev->onset+0.055));
				near = near->next) {
			if (near->ev == e->ev)
				continue;
			if ((abs(near->ev->pitch - e->ev->pitch) == 1) &&
					(near->ev->salience / e->ev->salience > 2)) {
				if (parameters->debug("spectrum", "showDeleted")) {
					cout << "Del(N):\n";
					e->ev->print();
					near->ev->print();
				}
				e->ev->volume = 0;	// mark for removal
				break;
			}
			if (isMidiHarmonic(e->ev->pitch - near->ev->pitch) &&
					((near->ev->volume / e->ev->volume > 0.5) ||
						(near->ev->salience / e->ev->salience > 2))) {
				if (parameters->debug("spectrum", "showDeleted")) {
					cout << "Del(H):\n";
					e->ev->print();
					near->ev->print();
				}
				e->ev->volume = 0;
				break;
			}
		}
		if (e->ev->volume == 0) {
			e = e->prev;
			e->remove(e->next->ev);
		}
	}
	if (parameters->debug("spectrum", "showEvents"))
		e->print();
	if (strcmp(parameters->getString("play","off"), "on") == 0)
		e->play();
	return e;
} // filterEvents()

void spectrogram :: psOut(char* filename) {
	psFile* f = new psFile(filename, fCount, tCount, fMin, tInc);
	for (int count = 0; count < tCount; count++)
		f->psData(data, count);
	f->psEnd();
} // psOut()


// Implementation of CLASS TRACKER

tracker :: tracker(double timeResn) {
	head = new eventList();
	aliveHead = head;
	hiThreshold = parameters->getDouble("hiThreshold", 1000.0);
	tInc = timeResn;
} // tracker constructor

void tracker :: print() {
	finalEvents()->print();
} // print()

void tracker :: add(double freq, double power, double time) {
	eventList* ptr;
	int midinum = midiNumber(freq);
	if ((midinum < 21) || (midinum > 108))
		return;
	if (aliveHead == head)
		aliveHead = aliveHead->next;
	while ((aliveHead->ev != NULL) && (aliveHead->ev->flags == DEAD))
		aliveHead = aliveHead->next;
	for (ptr = aliveHead; ptr->ev != NULL; ptr = ptr->next)
		if (ptr->ev->flags != DEAD) {
			if (time - ptr->ev->offset > 2.5 * tInc)
				ptr->ev->flags = DEAD;
			else if (fabs(ptr->ev->beat/ptr->ev->volume - freq)/freq < 0.03) {
				ptr->ev->offset = time;
				ptr->ev->beat += freq;	// sum of frequencies
				ptr->ev->volume++;		// counter (for average freq calcn
				if (ptr->ev->salience < power)
					ptr->ev->salience = power;
				if (parameters->debug("spectrum", "showTPV"))
					cout << setprecision(3)
						 << "added f=" << setw(8) << freq
						 << " p=" << setw(8) << power
						 << " t=" << setw(8) << time << " to existing trk\n";
				break;
			}
		}
	if (ptr->ev == NULL) {	// no track found, make a new one
		event* e = new event(time, time, 0, 1, freq);
		e->salience = power;
		e->flags = ALIVE;
		ptr->add(e);
		if (parameters->debug("spectrum", "showTPV"))
			cout << "added f=" << setw(8) << freq
				 << " p=" << setw(8) << power
				 << " t=" << setw(8) << time << " to new trk\n";
	}
} // add()

eventList* tracker :: finalEvents() {	// finalises list
	eventList* e;
	int cnt1 = 0;
	int cnt2 = 0;
	for (e = head->next; e->ev != NULL; e = e->next) {
		if (e->ev->flags == ALIVE) {
			cnt2++;
			e->ev->flags = DEAD;
		}
		if (e->ev->volume < 3) {
			cnt1++;
			e = e->prev;
			e->remove(e->next->ev);
		} else {
			e->ev->beat /= double(e->ev->volume);
			e->ev->pitch = midiNumber(e->ev->beat);
			e->ev->salience = sqrt(e->ev->salience);
			e->ev->volume = vol(e->ev->salience, hiThreshold);
			e->ev->offset += tInc;
		}
	}
	cout << "deleted " << cnt1 << " short tracks; found "
		 << cnt2 << " alive tracks" << endl;
	return head;
} // finalEvents()


// Implementation of CLASS PSFILE

psFile :: psFile(char* fileName, int fc, int tc, double fm, double tmInc):
	name(fileName), f(fileName), fCount(fc), tCount(tc), fMin(fm), tInc(tmInc) {
	if (!f.is_open())
		throw Exception("psFile(): opening file");
	length = 5;
	width = 5;	// 555 / fCount;
	pageSize = 150;
	min = parameters->getDouble("loThreshold", 10.0);
	max = parameters->getDouble("hiThreshold", 1000.0);
	pageCount = 1;
	lineCount = 0;
	psHeader();
} // psFile constructor

void psFile :: psHeader() {		// Note: A4 page = 595x842
	int xstart = 80;
	int ystart = 60;
	f << "%%!PS-Adobe-2.0\n%%%%Pages: (atend)\n"		// Minimal PS header
	  << "/f14 { /Palatino-Roman findfont 14 scalefont setfont } def\n"
	  << "/f10 { /Palatino-Roman findfont 10 scalefont setfont } def\n"
	  << "/vshow { 90 rotate show -90 rotate } def\n"
	  << "/xstart " << xstart << " def\n"
	  << "/ystart " << ystart << " def\n"
	  << "/xstop " << width * fCount + xstart << " def\n"
	  << "/ystop " << length * pageSize + ystart << " def\n"
	  << "/xsize " << width << " def\n"
	  << "/ysize " << length << " def\n"

	  << "/data { % (x y z) -> (x y)\n"					// plot one data point
	  << "\tsetgray\n\tnewpath    %% data point\n"
	  << "\t\t2 copy moveto\n"
	  << "\t\t0 ysize rlineto\n"
	  << "\t\txsize 0 rlineto\n"
	  << "\t\t0 0 ysize sub rlineto\n"
	  << "\tclosepath fill\n} def\n"

	  << "/row { % (x y d1 .. dn n) -> (x y')\n"		// plot data points
	  << "\tdup dup 3 add exch 1 add roll 3 2 roll\n"
	  << "\t{ 3 2 roll data exch xsize add exch} repeat\n"
	  << "\texch pop xstart exch ysize add\n} def\n"

	  << "/drawpage { % (pg zz zz) -> ()\n"				// draw axes/grid/labels
	  << "\tpop pop 0.0 setgray 25 20 moveto \n"
	  << "\tf14 (File: ) vshow (" << name << ") vshow\n"	// draw page labels
	  << "\t25 780 moveto (Page: ) vshow (   ) cvs vshow\n"
	  << "\txstop 35 add 300 moveto (Time (seconds)) vshow\n"
	  << "\t360 ystart 30 sub moveto (Pitch (semitones))\n"
	  << "\t180 rotate show -180 rotate\n"
	  << "\t2 setlinewidth\n"						// draw axes
	  << "\tnewpath    %% pitch-axis\n"
	  << "\t\txstart ystart moveto xstop ystart lineto\n"
	  << "\tclosepath stroke\n"
	  << "\tnewpath    %% time-axis\n"
	  << "\t\txstop ystart moveto xstop ystop lineto\n"
	  << "\tclosepath stroke\n"
	  << "\tf10\n";									// draw grids
	int coffset = (0 + 132 - midiNumber(fMin)) % 12;		// draw octave grid
	int eoffset = (4 + 132 - midiNumber(fMin)) % 12;
	int goffset = (7 + 132 - midiNumber(fMin)) % 12;
	// Xoffset is number of notes from fMin to first X
	f << "\t(G) " << (midiNumber(fMin) + goffset) / 12 - 2
	  << " xstart " << (fCount - goffset) * width
	  << " add ystart " << (fCount - goffset) / 12 + 1	// number of octaves
	  << " 1\n"
	  << "\t(E) " << (midiNumber(fMin) + eoffset) / 12 - 2
	  << " xstart " << (fCount - eoffset) * width
	  << " add ystart " << (fCount - eoffset) / 12 + 1	// number of octaves
	  << " 1\n"
	  << "\t(C) " << (midiNumber(fMin) + coffset) / 12 - 2
	  << " xstart " << (fCount - coffset) * width
	  << " add ystart " << (fCount - coffset) / 12 + 1	// number of octaves
	  << " 2\n"
	  << "\t3 {\n"
	  << "\t\tsetlinewidth\n"
	  << "\t\t{\n"
	  << "\t\t\tnewpath    %% pitch grid\n"
	  << "\t\t\t\t2 copy moveto 1 index ystop lineto\n"
	  << "\t\t\tclosepath stroke\n"
	  << "\t\t\t1 index 3 add ystart 18 sub moveto\n"
	  << "\t\t\t3 2 roll\n"
	  << "\t\t\t1 add dup (  ) cvs 4 index vshow vshow\n"
	  << "\t\t\t3 1 roll\n"
	  << "\t\t\texch " << 12 * width << " sub exch\n"
	  << "\t\t} repeat pop pop pop pop\n"
	  << "\t} repeat\n"
	  << "\txstart ystart\n"							// draw time grid
	  << "\t" << pageSize / 10 << " {\n"
	  << "\t\tnewpath    %% time grid\n"
	  << "\t\t\t" << 10 * length << " add 2 copy moveto xstop 1 index lineto\n"
	  << "\t\tclosepath fill\n"
	  << "\t\txstop 12 add 1 index 7 sub moveto\n"
	  << "\t\t4 2 roll 1 index add dup 10 mul round 10 div\n"
	  << "\t\t(     ) cvs vshow 4 2 roll\n"
	  << "\t\t" << 0 * length << " add\n\t} repeat pop pop pop pop\n"
	  << "\tshowpage\n"									// output page
	  << "} def\n"
	  << "%%%%EndProlog\n";
} // psHeader()

void psFile :: psTrailer() {
	f << "drawpage\n%%%%EndPage: " << pageCount << " " << pageCount << "\n"
	  << "%%%%Pages: " << pageCount << endl;
} // psTrailer()

void psFile :: psEnd() {
	psTrailer();
	f.close();
} // psEnd()

void psFile :: psData(double* data, int t) {
	if (lineCount == 0) {	// beginning of new page
		f << "%%%%Page: " << pageCount << " " << pageCount << "\n"
		  << setprecision(3) << tInc * 10 << " "
		  << setprecision(1) << tInc * 150 * (pageCount - 1) << " "
		  << pageCount << " xstart ystart\n";
	}
	for (int i = 0; i < fCount; i++) {
		f << setprecision(2)
		  << 1.0 - (threshold(data[i*tCount+t],min,max) - min) / (max-min)
		  << ((i % 12 == 11)? "\n" : " ");
	}
	f << "    " << fCount << " row\n";
	if (++lineCount >= pageSize) {
		f << "drawpage\n%%%%EndPage: " << pageCount << " " << pageCount << "\n";
		lineCount = 0;
		pageCount++;
	}
} // psData()
