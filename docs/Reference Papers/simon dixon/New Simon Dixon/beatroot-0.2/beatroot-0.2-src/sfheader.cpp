//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sfheader.cpp
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
#include "util.h"


void sfheader :: init(int ln, int rt, int ft, int ch,
						const char* txt, int txtln, const char* nm) {
	if (ln < 0)
		length = 0;
	else
		length = ln;
	rate = rt;
	format = ft;
	channels = ch;
	if (txtln) {
		text = new char[txtln];
		memcpy(text, txt, txtln);
	} else
		text = 0;
	textlength = txtln;
	if (nm) {
		filename = new char[strlen(nm)+1];
		strcpy(filename, nm);
	} else
		filename = 0;
	error = false;
} // init()

sfheader :: sfheader() {
	init(0, DEFAULT_RATE, DEFAULT_FORMAT, DEFAULT_CHANNELS, 0, 0, 0);
} // sfheader constructor

sfheader :: sfheader(const sfheader& orig) {
	init(orig.length, orig.rate, orig.format, orig.channels,
			orig.text, orig.textlength, orig.filename);
} // sfheader constructor

sfheader :: sfheader(int ln, int rt, int ch) {
	init(ln, rt, DEFAULT_FORMAT, ch, 0, 0, 0);
} // sfheader constructor

bool sfheader :: readSndHeader(ifstream &f, const char *name) {
	unsigned char tmp[24];
	f.read(tmp,24);
	if (!assertWarning(f.gcount() == 24,"sfheader: reading header") ||
		!assertWarning(strncmp((char*)tmp, ".snd", 4) == 0,
						"sfheader: not .snd file"))
		return false;
	init(sRead32BE(tmp+8) / (sizeof(audioSample) * sRead32BE(tmp+20)),
		 sRead32BE(tmp+16), sRead32BE(tmp+12), sRead32BE(tmp+20), 0, 0, name);
	if (!assertWarning(format==SUN_LINEAR_PCM, "sfheader: PCM format expected"))
		return false;
	textlength = sRead32BE(tmp+4) - 24;
	text = new char[textlength+1];
	text[textlength] = 0;
	f.read(text,textlength);
	if (!assertWarning(f.gcount() == textlength,"sfheader: reading text field"))
		return false;
	return true;
} // readSndHeader()

bool sfheader :: readWavHeader(ifstream &f, const char *name) {
	bool formatFound = false;
	unsigned char format[16];
	unsigned char data[12];
	f.read(data,12);
	if (!assertWarning(f.gcount() == 12,"Error reading header") ||
		!assertWarning(strncmp((char*)data, "RIFF", 4) == 0, "not RIFF file") ||
		!assertWarning(strncmp((char*)data+8, "WAVE", 4) == 0, "not WAV file"))
		return false;
	while (!f.eof()) {		// read chunks until data is found
		f.read(data,8);
		if (!assertWarning(f.gcount() == 8,"Error reading header"))
			return false;
		int chunkLength = sRead32LE(data+4);
		if (strncmp((char*)data, "data", 4) == 0) {
			if (!assertWarning(formatFound, "No format data in WAV file"))
				return false;
			init(chunkLength / (sizeof(audioSample) * sRead16LE(format+2)),
				 sRead32LE(format+4), sRead16LE(format), sRead16LE(format+2),
				 0, 0, name);
			return true;
		} else if (strncmp((char*)data, "fmt ", 4) == 0) {
			f.read(format,16);
			if (!assertWarning(f.gcount()==16, "Error in WAV format data") ||
				!assertWarning(sRead16LE(format) == WAV_LINEAR_PCM,
								"not WAV linear PCM format") ||
				!assertWarning(sRead16LE(format+14) == 16, "not 16 bit format"))
				return false;
			f.seekg(chunkLength-16, ios::cur);
			formatFound = true;
		} else
			f.seekg(chunkLength, ios::cur);
	}
	return assertWarning(false, "WAV data not found");
} // readWavHeader()

sfheader :: sfheader(ifstream& f, const char* name) {
	if (strcasecmp(".snd", name+strlen(name)-4) == 0)
		error = readSndHeader(f, name);
	else {
		assertWarning(strcasecmp(".wav", name+strlen(name)-4) == 0,
						"Bad extension: assuming WAV format");
		error = readWavHeader(f, name);
	}
} // sfheader constructor

sfheader :: ~sfheader() {
	if (text)
		delete [] text;
} // sfheader destructor

void sfheader :: print() const {
	if (filename)
		cout << "Filename: " << filename << endl;
	cout << "Length:   " << length << endl;
	cout << "Format:   " << format << endl;
	cout << "Rate:     " << rate << endl;
	cout << "Channels: " << channels << endl;
	if (text)
		cout << "Text:     " << text << endl;
} // print()
