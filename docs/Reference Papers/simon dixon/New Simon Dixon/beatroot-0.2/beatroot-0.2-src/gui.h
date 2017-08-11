//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: gui.h
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

#include <jni.h>

#define SHOW_AUDIO 1024		/* DO NOT CHANGE - see BeatTrackDisplay.java */
#define SHOW_SPECTRO 2048	/* DO NOT CHANGE - see BeatTrackDisplay.java */
#define SHOW_MIDI 4096		/* DO NOT CHANGE - see BeatTrackDisplay.java */

class eventList;
class sample;
class spectrogram;

class gui {

        JavaVM* jvm;					// Java virtual machine
        JNIEnv* env;					// Environment of JVM

		jobject jGUI;					// Hooks into the Java GUI
		jmethodID jSetAudioData;
		jmethodID jSetMidiData;
		jmethodID jSetBeatData;
		jmethodID jSetSpectroData;
		jmethodID jGetBeatData;
		jmethodID jGetRequest;
		jmethodID jShowMessage;
		jmethodID jSetBlindMode;

		eventList* midiData;			// Local data
		eventList* beatData;
		eventList* undoData;
		sample* audioData;
		sample* audioPlayData;
        spectrogram* spectroData;
		eventList* audioEnv;
		int mode;

		int jAssert(int check, const char* mesg, bool fatal=false);
		eventList* getBeatData();
		void updateBeatData();
		void updateBeatDisplay();
		void updateAudioDisplay();
		void updateMidiDisplay();
		void rememberBeatData();
		void restoreBeatData();
		void setMode(int on, int off);

    public:
		gui();
        ~gui();
		void processRequests();
		void showWarningDialog(const char* message);
};

