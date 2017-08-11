//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: playq.h
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

#define _REENTRANT    /* for threads */
#include <pthread.h>

#include "local.h"

class sample;

class playQ {
	pthread_t tid;

public:
	typedef enum {UNUSED, FILLING, READY, PLAYING, DONE} playQState;
	typedef struct {
		playQState state;
		bool isAudio;
		audioSample* startSample;
		int length;
		int rate;
		int channels;
		sample* owner;
		audioSample* deleteAddr;
		char* midiFileName;
	} entry;
	entry* elements;
	int current, toPlay, rate, channels, audioFD;
	playQState allState;
	int qSize;
	bool interrupted;

	int getCurrent();
	void openAudioDevice();
	void closeAudioDevice();

	playQ(int sz = 20);
	~playQ();
	void queueAudio(sample* s, audioSample* startPoint, int len);
	void queueMidi(char* fileName);
	bool notUsing(sample* s);
};

