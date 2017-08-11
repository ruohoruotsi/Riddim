//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: sfheader.h
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

#define SUN_LINEAR_PCM 3
#define WAV_LINEAR_PCM 1
#define DEFAULT_FORMAT WAV_LINEAR_PCM
#define DEFAULT_RATE 44100
#define DEFAULT_CHANNELS 1

class sfheader {
public:
	int length;
	int format;
	int rate;
	int channels;
	char* text;
	int textlength;
	char* filename;
	bool error;

	sfheader();
	sfheader(const sfheader& hdr);
	sfheader(int len, int rt, int ch = DEFAULT_CHANNELS);
	sfheader(ifstream& f, const char* name);
	~sfheader();

	void print() const;
private:
	void init(int len, int rate, int format, int channels,
			  const char* text, int textlen, const char* name);
	bool readSndHeader(ifstream& f, const char* name);
	bool readWavHeader(ifstream& f, const char* name);
};
