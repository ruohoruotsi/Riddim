//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: tmf.h
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

#ifndef INCLUDE_TMF_H
#define INCLUDE_TMF_H

#include "includes.h"

typedef enum {
	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
	KEY_AFTERTOUCH = 0xA0,
	CONTROLLER_CHANGE = 0xB0,
	PROGRAM_CHANGE = 0xC0,
	CHANNEL_AFTERTOUCH = 0xD0,
	PITCH_WHEEL = 0xE0,
	SYSTEM_EXCLUSIVE = 0xF0,
	META_EVENT = 0xFF
} MidiMessage;

typedef enum {
	SEQUENCE_NUMBER = 0x00,
	TEXT_EVENT = 0x01,
	COPYRIGHT_NOTICE = 0x02,
	SEQUENCE_NAME = 0x03,
	INSTRUMENT_NAME = 0x04,
	LYRIC = 0x05,
	MARKER = 0x06,
	CUE_POINT = 0x07,
	CHANNEL_PREFIX = 0x20,
	END_OF_TRACK = 0x2F,
	SET_TEMPO = 0x51,
	SMPTE_OFFSET = 0x54,
	TIME_SIGNATURE = 0x58,
	KEY_SIGNATURE = 0x59,
	SEQUENCER_SPECIFIC = 0x7F
} MetaMessage;

typedef enum {
	DAMPER_PEDAL = 0x40,
	PORTAMENTO = 0x41,
	SOSTENUTO_PEDAL = 0x42,
	SOFT_PEDAL = 0x43,
	GENERAL_4 = 0x44,
	HOLD_2 = 0x45,
	GENERAL_5 = 0x50,
	GENERAL_6 = 0x51,
	GENERAL_7 = 0x52,
	GENERAL_8 = 0x53,
	TREMOLO_DEPTH = 0x5C,
	CHORUS_DEPTH = 0x5D,
	DETUNE = 0x5E,
	PHASER_DEPTH = 0x5F,
	DATA_INCREMENT = 0x60,
	DATA_DECREMENT = 0x61,
	NON_REG_LSB = 0x62,
	NON_REG_MSB = 0x63,
	REG_LSB = 0x64,
	REG_MSB = 0x65
} ControllerMessage;

inline bool isPedalOn(int x) { return x >= 0x40; }
inline bool isNoteOn(const char* x) { return (strcasecmp(x,"on") == 0); }
inline bool isNoteOff(const char* x) { return (strcasecmp(x,"off") == 0); }
inline bool isKeyAfterTouch(const char* x) {
	return ((strcasecmp(x,"popr") == 0) || (strcasecmp(x,"polypr") == 0)); }
inline bool isChannelAfterTouch(const char* x) { 
	return ((strcasecmp(x,"chpr") == 0) || (strcasecmp(x,"chanpr") == 0)); }
inline bool isControllerChange(const char* x) {
    return ((strcasecmp(x,"par") == 0) || (strcasecmp(x,"param") == 0)); }
inline bool isPitchBend(const char* x) { return (strcasecmp(x,"pb") == 0); }
inline bool isProgramChange(const char* x) {
    return ((strcasecmp(x,"prch") == 0) || (strcasecmp(x,"progch") == 0)); }
inline bool isSysex(const char* x) { return (strcasecmp(x,"sysex") == 0); }
inline bool isArbitrary(const char* x) { return (strcasecmp(x,"arb") == 0); }
inline bool isMeta(const char* x) { return (strcasecmp(x,"meta") == 0); }
inline bool isSeqSpec(const char* x) { return (strcasecmp(x,"seqspec") == 0); }
inline bool isSeqNum(const char* x) { return (strcasecmp(x,"seqnr") == 0); }
inline bool isKeySig(const char* x) { return (strcasecmp(x,"keysig") == 0); }
inline bool isTempo(const char* x) { return (strcasecmp(x,"tempo") == 0); }
inline bool isTimeSig(const char* x) { return (strcasecmp(x,"timesig") == 0); }
inline bool isSMPTE(const char* x) { return (strcasecmp(x,"smpte") == 0); }
inline bool isTrackEnd(const char* x) { return(strcasecmp(x,"trkend") == 0); }
inline bool isText(const char* x) { return(strcasecmp(x,"text") == 0); }
inline bool isCopyright(const char* x) { return(strcasecmp(x,"copyright")==0); }
inline bool isSeqName(const char* x) { return(strcasecmp(x,"seqname") == 0); }
inline bool isTrkName(const char* x) { return(strcasecmp(x,"trkname") == 0); }
inline bool isInstrName(const char* x) { return(strcasecmp(x,"instrname")==0); }
inline bool isLyric(const char* x) { return(strcasecmp(x,"lyric") == 0); }
inline bool isMarker(const char* x) { return(strcasecmp(x,"marker") == 0); }
inline bool isCue(const char* x) { return(strcasecmp(x,"cue") == 0); }
inline bool isMetaOther(const char* x) { return(strncasecmp(x,"0x", 2) == 0); }
inline bool isMinor(const char* x) { return(strncasecmp(x,"minor", 2) == 0); }


class event;
class eventList;
class eventTree;

class InputMIDIBase {

private:
	const char* fileName;
	int clockOffset;
	double timeOffset;
	double clockFactor;

protected:
	ifstream in;
	int trackCount;
	int format;         // MIDI format (0,1,2)
	int midiDivision;
	eventTree* events;
	eventList* tempoMap;
	bool warned;
	eventList* listEvents;

	virtual void readTrack(int trackNumber) = 0;
	double intToTime(int midiTime);
	void resetClock();
	void setTempo(event* e);
	void addEvent(event* e);

public:
	InputMIDIBase(const char* nm, eventList* listEvts = NULL);
	eventList* getContents() { return listEvents; }
};

class iMIDI : public InputMIDIBase {

protected:
	void readTrack(int trackNumber);
	int getVarLength();
	int getFixLength(int len);
	int getByte() { return getFixLength(1); }
	int getShort() { return getFixLength(2); }
	int getInt() { return getFixLength(4); }
	void getData(int len, event* e);
	int byteCount;

public:
	iMIDI(const char* nm, eventList* listEvts = NULL);
};

class iTMF : public InputMIDIBase {

protected:
	void readTrack(int trackNumber);
	bool addMetaEvent(int midiTime, int trackNumber);

	int getNoteNumber(char c);
	int getVal(char*);
	int getChannel();
	void getData(event* e);
	void getHex(event* e);
	void getString(event* e);
	inline int getNote()       { return getVal("note"); }
	inline int getVelocity()   { return getVal("vol");  }
	inline int getValue()      { return getVal("val");  }
	inline int getController() { return getVal("con");  }
	inline int getProgram()    { return getVal("prog"); }

public:
	iTMF(const char* nm, eventList* listEvts = NULL);
}; // class iTMF

class OutputMIDIBase {

private:
	const char* fileName;
	int clockOffset;
	double timeOffset, timeFactor;

protected:
	eventList* events;
	double start, length;
	ofstream out;
	int format;			// MIDI format (0,1,2)
	int trackCount;
	int head, midiDivision, midiTempo;
	double previousEvent;
	static const int MIDI_QUEUE_SIZE = 1000;
	const event* eventQ[MIDI_QUEUE_SIZE];	// queue; ordered latest -> earliest

	int timeToInt(double time);
	void resetClock();
	void setTempo(double time, int newTempo);
	void writeTrack(int trackNumber);
	virtual void startTrack(int trackNumber, bool hasTempo) = 0;
	virtual void endTrack() = 0;
	virtual void putEvent(const event* ev) = 0;
	virtual void flushQueue(double time) = 0;

public:
	OutputMIDIBase(const char* nm, eventList* e, double start=0, double len=-1);

}; // class OutputMIDIBase

class oMIDI: public OutputMIDIBase {

	int deltaMarker;
	int lengthPointer;
	int deltaTime(int midiTime);
	void putVlen(int value);
	virtual void startTrack(int trackNumber, bool hasTempo);
	virtual void endTrack();
	virtual void putEvent(const event* ev);
	virtual void flushQueue(double time);

public:
	oMIDI(const char* name, eventList* e, double start = 0, double len = -1);

};

class oTMF: public OutputMIDIBase {

	virtual void startTrack(int trackNumber, bool hasTempo);
	virtual void endTrack();
	virtual void putEvent(const event* ev);
	virtual void flushQueue(double time);

public:
	oTMF(const char* name, eventList* e, double start = 0, double len = -1);

};
	
#endif
