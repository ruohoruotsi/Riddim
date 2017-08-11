//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: song.cpp
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
#include "sample.h"
#include "song.h"
#include "util.h"


song :: song(const char *filename, int iT, int iP, int fT, int fP) {
	name = filename;
	initialTempo = iT;
	initialPhase = iP;
	finalTempo = fT;
	finalPhase = fP;
}

song :: song(const char *filename) {
	ifstream f(filename);
	name = filename;
	if (!f.is_open())
		throw Exception("song: opening file");
	sfheader hdr(f, filename);
	if (hdr.error)
		throw Exception("song: error in header");
	if (hdr.textlength != 24)
		throw Exception("song: textlength wrong");
	if (strcmp(hdr.text,"beaTmix") != 0)
		throw Exception("song: not beaTmix file");
	initialTempo = sRead32BE((unsigned char*)hdr.text+8);
	initialPhase = sRead32BE((unsigned char*)hdr.text+12);
	finalTempo = sRead32BE((unsigned char*)hdr.text+16);
	finalPhase = sRead32BE((unsigned char*)hdr.text+20);
	assertWarning(abs(initialTempo-finalTempo) < 2000, "Large tempo variation");
	if( (initialPhase < 0) || (initialPhase > finalPhase) ||
			(finalPhase > hdr.length))
		throw Exception("song: beaTmix data (beat posns)");
	double bpm = double(hdr.rate) / double(initialTempo) * 60.0;
	if ((bpm < 60) || (bpm > 240))
		throw Exception("song: beaTmix data (initialTempo)");
	bpm = double(hdr.rate) / double(finalTempo) * 60.0;
	if ((bpm < 60) || (bpm > 240))
		throw Exception("song: beaTmix data (finalTempo)");
}

void song :: write(const char *filename) const {
	sample snd(name);
	snd.annotate(initialTempo, initialPhase, finalTempo, finalPhase);
	snd.writeSndFile(filename);
}

void playSongList(char *songList[]) {
	song *prev, *next;
	int start, stop, newRate;
	const int crossfadeBeats = 8;
	int songNumber = 0;

	// play first song (except ending)
	if (parameters->debug("song", "basic"))
		cerr << "Playing initial sample" << endl;
	if (songList[songNumber] == NULL)
		throw Exception("playSongList: empty list");
	prev = new song(songList[songNumber++]);
	start = 0;
	stop = prev->finalPhase - crossfadeBeats * prev->finalTempo;
	sample initialStart(prev->name, start, stop);
	initialStart.play();

	while (songList[songNumber]) {
		{ // crossfade previous song with next
		if (parameters->debug("song", "basic"))
			cerr << "Calculating crossfade" << endl;
		sample fadeOut(prev->name, stop, prev->finalPhase);
		next = new song(songList[songNumber++]);
		start = next->initialPhase;
		stop = next->initialPhase + crossfadeBeats * next->initialTempo;
		sample fadeIn(next->name, start, stop);
		newRate = fadeOut.rate() * prev->finalTempo / next->initialTempo;
		sample* temp = fadeIn.changeRate(newRate);
		sample* crossfadeSound = fadeOut.crossfade(*temp);
		crossfadeSound->play();
		}

		{ // restore new song to normal tempo
		if (parameters->debug("song", "basic"))
			cerr << "Calculating warp" << endl;
		start = stop;
		stop += 100 * abs(prev->finalTempo - next->initialTempo);
		sample preWarpSound(next->name, start, stop);
		sample *warpSound = preWarpSound.warpRate(newRate);
		warpSound->play();
		}

		{ // play rest of new song (except ending)
		if (parameters->debug("song", "basic"))
			cerr << "Playing song body" << endl;
		start = stop; 
		stop = next->finalPhase - crossfadeBeats * next->finalTempo;
		sample songBody(next->name, start, stop);
		songBody.play();
		delete prev;
		prev = next;
		}
	}

	// play ending of last song
	if (parameters->debug("song", "basic"))
		cerr << "Playing final ending" << endl;
	sample finalEnding(prev->name, stop, prev->finalPhase);
	finalEnding.play();
	delete prev;
} // playSongList()
