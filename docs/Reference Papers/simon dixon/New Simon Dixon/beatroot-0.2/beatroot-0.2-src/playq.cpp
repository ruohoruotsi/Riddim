//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: playq.cpp
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

#define TIMIDITY "/raid/music/bin/timidity"

void *playThread(playQ *p);

playQ :: playQ(int sz) {
	qSize = sz;
	audioFD = -1;
	elements = new entry[sz];
	for (int i = 0; i < qSize; i++)
		elements[i].state = playQ :: UNUSED;
	allState = playQ :: UNUSED;
	toPlay = current = 0;
	if (parameters->debug("playq", "basic"))
		cerr << "playQ initialised\n";
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	struct sched_param param;
	pthread_attr_getschedparam(&attr,&param);
	param.sched_priority = 100;	// default 0
	pthread_attr_setschedparam(&attr,&param);
	assertWarning(pthread_create(&tid, &attr, playThread, this) == 0,
		"Unable to create thread for playQ");
} // playQ constructor

playQ :: ~playQ() {
	while (elements[current].state != playQ :: UNUSED) {
		if (parameters->debug("playq", "basic"))
			cerr << "Waiting for play queue to empty\n";
		sleep(1);
	}
	elements[current].state = playQ :: DONE;
	pthread_join(tid, NULL); // wait for thread to finish
} // playQ destructor

int playQ :: getCurrent() {
	if (parameters->debug("playq","basic") && (elements[current].state!=UNUSED))
		cerr << "Waiting to allocate buffer\n";
	while (elements[current].state != UNUSED)
		sleep(1);   //wait for empty posn in queue
	int save = current;
	if (++current == qSize)
		current = 0;
	return save;
} // getCurrent()

void playQ :: queueAudio(sample* sam, audioSample* data, int len) {
	int posn = getCurrent();
	elements[posn].isAudio = true;
	elements[posn].startSample = data;
	elements[posn].length = len;
	elements[posn].rate = sam->rate();
	elements[posn].channels = sam->channels();
	elements[posn].owner = sam;
	elements[posn].deleteAddr = NULL;
	elements[posn].state = READY;
} // queueAudio()

void playQ :: queueMidi(char* fileName) {
	int posn = getCurrent();
	elements[posn].isAudio = false;
	elements[posn].owner = NULL;
	elements[posn].midiFileName = new char[strlen(fileName)+1];
	strcpy(elements[posn].midiFileName, fileName);
	elements[posn].state = READY;
} // queueMidi()

bool playQ :: notUsing(sample* s) {
	for (int ptr = current-1; TRUE; ptr--) {
		if (ptr < 0)
			ptr = qSize - 1;
		if (elements[ptr].state == UNUSED)
			return true;
		if (elements[ptr].owner == s) {
			elements[ptr].deleteAddr = s->data;
			elements[ptr].owner = NULL;
			return false;
		}
	}
} // notUsing()

void playQ :: openAudioDevice() {
	if (parameters->debug("playq", "audioDevice"))
		cerr << "(Re)opening audio device" << endl;
	int format = AUDIO_FORMAT;
	rate = elements[toPlay].rate;
	channels = elements[toPlay].channels;
	if (!assertWarning((audioFD = open("/dev/audio",O_WRONLY | O_NONBLOCK)) > 2,
			"Unable to open audio device"))
		return;
	int checkFormat = format;
	int checkRate = rate;
	int checkChannels = channels;
	if (!assertWarning(!ioctl(audioFD, SNDCTL_DSP_SETFMT, &checkFormat),
			"Error setting audio format") ||
		!assertWarning(format == checkFormat, "Audio format not supported") ||
		!assertWarning(!ioctl(audioFD, SNDCTL_DSP_CHANNELS, &checkChannels),
			"Error setting audio channels") ||
		!assertWarning(channels == checkChannels, "Channel selection failed") ||
		!assertWarning(!ioctl(audioFD, SNDCTL_DSP_SPEED, &checkRate),
			"Error setting sample rate") ||
		// +-100Hz tolerance:  kludge because soundcard clock resolution >> 1Hz
		!assertWarning(abs(rate-checkRate) < 100, "Unsupported sample rate")) {
			close(audioFD);
			audioFD = -1;
	}
} // openAudioDevice()

void playQ :: closeAudioDevice() {
	audio_buf_info abi;
	do {			// let buffer drain before closing the device
		if (!assertWarning(!ioctl(audioFD, SNDCTL_DSP_GETOSPACE, &abi),
				"Unable to query audio status"))
			break;
		mySleep(200000);
	} while (abi.fragments < abi.fragstotal);
	if (parameters->debug("playq", "audioDevice"))
		cerr << "Closing audio device" << endl;
	assertWarning(close(audioFD) == 0, "Unable to close audio device");
	audioFD = -1;
} // closeAudioDevice

void interruptIgnorer(int signum) {
	if (parameters->debug("playq", "interrupts"))
		cerr << "Ignored signal: " << signum << endl;
} // interruptIgnorer()

void interruptHandler(int signum) {
	if (parameters->debug("playq", "interrupts"))
		cerr << "Interrupted by signal: " << signum << endl;
	playq->interrupted = true;
} // interruptHandler()

void *playThread(playQ *p) {
	char* timidity = parameters->getString("timidity", TIMIDITY);
	if (p->allState != playQ :: UNUSED) {
		if (parameters->debug("playq", "basic"))
			cerr << "playThread running\n";
		return NULL;
	}
	struct sigaction act = { interruptHandler, SA_NOMASK, SA_RESETHAND, 0 };
	struct sigaction ign = { interruptIgnorer, SA_NOMASK, SA_RESETHAND, 0 };
	sigaction(SIGINT, &act, 0);
	sigaction(SIGCHLD, &ign, 0);
	if (parameters->debug("playq", "basic"))
		cerr << "Audio thread started successfully\n";
	while (true) {
		if (p->elements[p->toPlay].state != playQ :: READY) {
			p->allState = playQ :: FILLING;
			if (parameters->debug("playq", "basic"))
				cerr << "Waiting for audio data in buffer " << p->toPlay <<endl;
		}
		while (p->elements[p->toPlay].state != playQ :: READY) {
			if (p->audioFD >= 0)
				p->closeAudioDevice();
			sleep(1);
			if (p->elements[p->toPlay].state == playQ :: DONE) {
				if (parameters->debug("playq", "basic"))
					cerr << "Audio thread completed\n";
				return NULL;
			}
		}
		if ((p->audioFD >= 0) && ((p->elements[p->toPlay].rate != p->rate) ||
							  (p->elements[p->toPlay].channels != p->channels)||
							  ! p->elements[p->toPlay].isAudio))
			p->closeAudioDevice();
		if (p->elements[p->toPlay].isAudio && (p->audioFD < 0))
			p->openAudioDevice();
		p->allState = playQ :: PLAYING;
		p->elements[p->toPlay].state = playQ :: PLAYING;
		p->interrupted = false;
		if (p->elements[p->toPlay].isAudio && (p->audioFD >= 0)) {
			audioSample *ptr = p->elements[p->toPlay].startSample;
			int sz = p->elements[p->toPlay].length;
			int writeSize = p->channels * sizeof(audioSample);
			int bytesWritten = 0;
			int i;
			for (i = 0; (i < sz) && !p->interrupted; ) {
				bytesWritten = write(p->audioFD, ptr, writeSize);
				if (bytesWritten == writeSize) {
					i++;
					ptr += p->channels;
				} else { // buffer must be full
					if (parameters->debug("playq", "audioAll"))
						cerr << bytesWritten << ": write to audio dev failed\n";
					mySleep(100000);
				}
				if (parameters->debug("playq", "audioAll")) {
					cerr << "Samples written: " << i + 1 << "   ";
					cerr << "Samples remaining: " << sz - i - 1 << endl;
				}
			}
			if (p->interrupted) {
				if (parameters->debug("playq", "interrupts"))
					cerr << "Audio interrupted" << endl;
				assertWarning(!ioctl(p->audioFD, SNDCTL_DSP_RESET),
						"Unable to reset audio device");	// clear buffer
			}
			strstream buf;
			buf << "audioCurrentPosition=" << i;
			parameters->add(buf.str());
			buf.freeze(0);
			if (p->elements[p->toPlay].deleteAddr != NULL)
				delete [] p->elements[p->toPlay].deleteAddr;
		} else if (!p->elements[p->toPlay].isAudio) {	// play midi file
			int pid = fork();
			assertWarning(pid >= 0, "playThread(): fork() failed");
			if (pid == 0) {		// separate process for timidity
				mySleep(400000);	// wait for parent to listen for SIGCHLD
				// ofstream blackHole("/dev/null");
				// cout = blackHole;	// ignore timidity output
				// cerr = blackHole;	// ignore timidity output
				execl(timidity, "timidity", "-EFreverb=0",
						p->elements[p->toPlay].midiFileName, 0);
				assertWarning(false, "playThread(): couldn't run timidity");
			} else if (pid > 0) { // main process monitors & interrupts timidity
				mySleep(100000);	// wait for child to be created
				sigaction(SIGCHLD, &act, 0);	// listen to child state change
				while (! p->interrupted) {		// wait for child or gui signal
					mySleep(100000);
				}
				kill(pid, SIGINT);				// kill child (if gui signal)
			}
			sigaction(SIGCHLD, &ign, 0);		// ignore further child signals
			string rm("/bin/rm ");
			rm += p->elements[p->toPlay].midiFileName;
			system(rm.c_str());							// remove the file
			delete p->elements[p->toPlay].midiFileName;
		}
		p->elements[p->toPlay].state = playQ :: UNUSED;
		if (++p->toPlay == p->qSize)
			p->toPlay = 0;
	}
} // playThread()
