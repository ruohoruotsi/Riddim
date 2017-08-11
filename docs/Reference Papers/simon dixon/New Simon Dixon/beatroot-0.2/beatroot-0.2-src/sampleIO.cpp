//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sampleIO.cpp
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

#include "param.h"
#include "playq.h"
#include "sample.h"
#include "util.h"


void sample :: print() const {
	info.print();
	if (parameters->debug("sampleIO", "basic")) {
		cout << "Data: " << data << endl;
	}
} // print()

void sample :: writeSndFile(const char *filename, int offset, int size) {
	if (!assertWarning((offset >= 0) && (offset < audioSize()),
						"Could not write sound file: illegal start"))
		return;
	int endCount = size < 0? audioSize(): size + offset;
	if (endCount > audioSize())
		endCount = audioSize();
	ofstream f(filename);
	if (!assertWarning(f.is_open(), "writeSndFile: could not open file"))
		return;
	f.write(".snd", 4);
	if (!info.text)
		info.textlength = 0;
	if (info.textlength < 4)
		write32BE(28, f);
	else
		write32BE(24 + info.textlength, f);
	write32BE((endCount - offset) * 2, f);	// assume 16 bit samples
	write32BE(SUN_LINEAR_PCM, f);
	write32BE(info.rate, f);
	write32BE(info.channels, f);
	if (info.textlength)
		f.write(info.text, info.textlength);
	for (int i=4; i > info.textlength; i--)
		f.write("", 1);
	for (int i=offset; i < endCount; i++)
		write16BE(data[i], f);				// assume 16 bit samples
	f.close();
} // writeSndFile()

void sample :: writeWavFile(const char *filename, int offset, int size) {
	// note: doesn't store tempo/phase information, just the audio data
	if (!assertWarning((offset >= 0) && (offset < audioSize()),
						"Could not write sound file: illegal start"))
		return;
	int endCount = size < 0? audioSize(): size + offset;
	if (endCount > audioSize())
		endCount = audioSize();
	int dataBytes = (endCount - offset) * 2;// assume 16 bit samples
	ofstream f(filename);
	if (!assertWarning(f.is_open(), "writeWavFile: could not open file")) 
		return;
	f.write("RIFF",4);
	write32LE(dataBytes + 36, f);			// file length - 8
	f.write("WAVEfmt ",8);
	write32LE(16, f);						// format chunk length
	write16LE(WAV_LINEAR_PCM, f);
	write16LE(info.channels, f);
	write32LE(info.rate, f);
	write32LE(dataBytes, f);
	write16LE(2, f);						// assume 16 bit samples
	write16LE(16, f);						// assume 16 bit samples
	f.write("data", 4);
	write32LE(dataBytes, f);
	for (int i=offset; i < endCount; i++)
		write16LE(data[i], f);				// assume 16 bit samples
	f.close();
} // writeWavFile()

void sample :: writeFile(char *filename, double start, double len) {
	if (!assertWarning((strlen(filename) > 4) && strrchr(filename, '.') && 
			(strrchr(filename, '.') - filename == strlen(filename) - 4),
					"Could not write file: illegal filename or extension"))
		return;
	int offset = (int)rint(start * rate() * channels());
	int size = ((len < 0)? -1: (int)rint(len * rate() * channels()));
	fade(-parameters->getInt("fadeIn", 0), offset);
	fade(parameters->getInt("fadeOut", 0), size < 0? -1: offset + size);
	if (!strcmp(parameters->getString("format", ""), "wav"))
		strcpy(filename+strlen(filename)-3, "wav");
	if (!strcmp(parameters->getString("format", ""), "snd"))
		strcpy(filename+strlen(filename)-3, "snd");
	if (endsWith(filename, ".wav", true))
		writeWavFile(filename, offset, size);
	else if (endsWith(filename, ".snd", true) || (info.format==SUN_LINEAR_PCM))
		writeSndFile(filename, offset, size);
	else {
		assertWarning(info.format == WAV_LINEAR_PCM, "Saving in WAV format");
		writeWavFile(filename, offset, size);
	}
} // writeFile()

float* sample :: toFloatArray() const {
	float* out = new float[audioSize()];
	for (int i = audioSize()-1; i >= 0; i--)
		out[i] = (float)data[i] / 32768.0;
	return out;
} // toFloatArray()

double* sample :: toDoubleArray() const {
	double* out = new double[audioSize()];
	for (int i = audioSize()-1; i >= 0; i--)
		out[i] = (double)data[i] / 32768.0;
	return out;
} // toDoubleArray()

void sample :: play(double start, double len) {
	int istart = (int)rint((double)rate() * start);
	int ilen = (int)rint((double)rate() * len);
	if (!assertWarning((istart >= 0) && (istart <= length()),
						"play(): illegal start time"))
		return;
	if ((ilen < 0) || (istart + ilen > length()))
		ilen = length() - istart;
	playq->queueAudio(this, data + (istart * channels()), ilen);
} // play()

void sample :: iPlay() {	// interactive playing of sample
	double startTime, durationTime;
	int i, startSample=0, durationSample=-1;
	const int bufSize = 256;
	char buf[bufSize];
	sample* tmp = NULL;
	play(0, 0);	// to initialise signal handler
	while (1) {
		cout << "Enter start time and length: " << flush;
		cin.getline(buf, bufSize);
		raise(SIGINT);		// stop playing previous segment
		enum {repeatPlay, newPlay, noPlay} playFlag = newPlay;
		switch (buf[0]) {
			case 'c':	// continue
				i = parameters->getInt("audioCurrentPosition", -1);
				if (i >= 0)
					startSample = i;
				if (i >= rate())
					startSample -= rate();	// 1 sec correction
				break;
			case 'd':	// double time
				delete tmp;	// it won't be playing now
				tmp = changeRate(2 * rate(), startSample, durationSample);
				tmp->setRate(rate());
				tmp->play();
				playFlag = noPlay; // or else it will kill the above play
				break;
			case '-':
			case 'p':	// previous
				if (isdigit(buf[1]))
					i = atoi(buf+1);
				else
					for (i = 1; (buf[i] == '-') || (buf[i] == 'p'); i++)
						;
				startSample -= i * rate();
				buf[0] = 'x';
				break;
			case '+':
			case 'n':	// next
				if (isdigit(buf[1]))
					i = atoi(buf+1);
				else
					for (i = 1; (buf[i] == '+') || (buf[i] == 'n'); i++)
						;
				startSample += i * rate();
				buf[0] = 'x';
				break;
			case 'h':	// help!
				cout << "<n1> [<n2>] = play from t=n1 to t=n1+n2\n"
						"<empty> = play again\n"
						"{p+}[<t1>] = go back t1 sec (default 1)\n"
						"{n-}[<t1>] = go forward t1 sec (default 1)\n"
						"w = where was i?\n"
						"c = continue from where it was stopped\n"
						"q = quit\n"
						"h = help\n"
						"<other> = stop playing" << endl;
				playFlag = noPlay;
				break;
			case 'w':	// where am i?
				mySleep(100000);
				i = parameters->getInt("audioCurrentPosition", -1);
				if (i >= 0)
					cout << "Stopped at: " << setprecision(2)
						<< rate() << "XXX" << i << "YYY" << startSample << "ZZZ"
						 << (double(i + startSample) / rate()) << endl;
				playFlag = noPlay;
				break;
			case 'q':	// quit
				return;
			case 0:
				playFlag = repeatPlay;
				break;
			default:	// stop
				if (!isdigit(buf[0]))
					playFlag = noPlay;
				break;
		}
		if (playFlag != noPlay) {
			istrstream in(buf);
			if (in >> startTime) {
				startSample = int(rate() * startTime);
				if (in >> durationTime)
					durationSample = int(rate() * durationTime);
			}
			if ((tmp != NULL) && (playFlag == repeatPlay)) // (buf[0] == 0))
				tmp->play();
			else {
				if (tmp != NULL) {
					delete(tmp);
					tmp = NULL;
				}
				if (startSample < 0)
					startSample = 0;
				if (startSample > length())
					startSample = length();
				double dstart = double(startSample) / double(rate());
				double dlen = double(durationSample) / double(rate());
				cout << setprecision(2) << "Playing: "
					 << dstart << " + " << dlen << endl;
				play(dstart, dlen);
			}
		}
	}
} // iPlay()
