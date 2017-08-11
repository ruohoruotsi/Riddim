//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: tmf.cpp
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
#include "tmf.h"
#include "event.h"
#include "param.h"
#include "tree.h"
#include "util.h"

InputMIDIBase :: InputMIDIBase(const char* nm, eventList* e):
		fileName(nm), in(nm), events(NULL),
		tempoMap(new eventList()), warned(false), listEvents(e) {
	if (!in.is_open())
		throw Exception("Could not open file for input");
} // InputMIDIBase constructor

// Class for input of MIDI format data

iMIDI :: iMIDI(const char* nm, eventList* e): InputMIDIBase(nm, e) {
	if (!matchString(in, "MThd") || (getFixLength(4) != 6))
		throw Exception("MIDI file header not found");
	format = getFixLength(2);
	trackCount = getFixLength(2);
	midiDivision = getFixLength(2);
	for (int trackNumber = 1; trackNumber <= trackCount; trackNumber++)
		readTrack(trackNumber);
	if (!listEvents)
		listEvents = new eventList();
	if (events) {
		events->addContents(listEvents);
		// listEvents->applyPedalEvents();
	}
} // iMIDI constructor

int iMIDI :: getVarLength() {
	unsigned char nextByte;
	int result = 0;
	do {
		if (! in.read(&nextByte,1))
			throw Exception("Read error in getVLength()");
		result = (result << 7) + (nextByte & 0x7F);
		byteCount++;
	} while (nextByte & 0x80);
	return result;
} // getVarLength()

int iMIDI :: getFixLength(int len) {
	unsigned char nextByte;
	int result = 0;
	for (int i = 0; i < len; i++) {
		if (! in.read(&nextByte,1))
			throw Exception("Read error in getVLength()");
		result = (result << 8) + nextByte;
		byteCount++;
	}
	return result;
} // getFixLength()

void iMIDI :: getData(int len, event* e) {
	e->data = new char[len+1];
	in.read(e->data, len);
	e->data[len] = 0;
	byteCount += len;
	e->flags = len;
} // getData()

void iMIDI :: readTrack(int trackNumber) {
	int time = 0;
	int command, channel, note, velocity, value;
	event* current;
	if (!matchString(in, "MTrk"))
		throw Exception("Parse error: no MTrk");
	event* currentNote[16][128] = { { NULL } };
	int voice[16] = { -1 };
	resetClock();
	int trackLength = getFixLength(4);
	byteCount = 0;
	while (byteCount < trackLength) {
		time += getVarLength();
		command = getByte();
		channel = command & 0xF;
		command &= 0xF0;;
		if (parameters->debug("tmf", "iMIDI"))
			cerr << time << " " << hex << command << dec << " "<<channel<<"\n";
		switch (command) {
		case NOTE_ON:
			note = getByte();
			velocity = getByte();
			if (currentNote[channel][note] != NULL) {
				if (velocity == 0) {	// old style note off
					currentNote[channel][note]->offset =
						currentNote[channel][note]->noteOff = intToTime(time);
					currentNote[channel][note] = NULL;
				} else
					assertWarning(false, "Double NoteOn in MIDI data");
			} else {
				current = new event(NOTE_ON, intToTime(time), note, velocity,
									voice[channel], trackNumber, channel);
				addEvent(current);
				currentNote[channel][note] = current; 
			}
			break;
		case NOTE_OFF:
			note = getByte();
			velocity = getByte();
			if (currentNote[channel][note] == NULL)
				assertWarning(false, "NoteOff without NoteOn");
			else {
				currentNote[channel][note]->offset = intToTime(time);
				currentNote[channel][note]->noteOff = intToTime(time);
				currentNote[channel][note] = NULL;
			}
			break;
		case KEY_AFTERTOUCH:
			note = getByte();
			value = getByte();
			addEvent(new event(KEY_AFTERTOUCH, intToTime(time), note, value,
								voice[channel], trackNumber, channel));
			break;
		case CHANNEL_AFTERTOUCH:
			value = getByte();
			addEvent(new event(CHANNEL_AFTERTOUCH, intToTime(time), value,value,
								voice[channel], trackNumber, channel));
			break;
		case CONTROLLER_CHANGE:
			note = getByte();
			value = getByte();
			addEvent(new event(CONTROLLER_CHANGE, intToTime(time), note, value,
					 voice[channel],trackNumber,channel));
			break;
		case PITCH_WHEEL:
			value = getByte();
			addEvent(new event(PITCH_WHEEL, intToTime(time), value, value,
								voice[channel], trackNumber, channel));
			break;
		case PROGRAM_CHANGE:
			value = getByte();
			voice[channel] = value;
			addEvent(new event(PROGRAM_CHANGE, intToTime(time), value, value,
								value, trackNumber, channel));
			break;
		case SYSTEM_EXCLUSIVE:
			if (channel == 0) {		// SysEx, not Meta
				current = new event(SYSTEM_EXCLUSIVE,intToTime(time),0,0,0,0,0);
				getData(getVarLength(), current);
				addEvent(current);
			} else if (channel == 15) {		// Meta events
				command = getByte();
				int length = getVarLength();
				switch (command) {
				case SET_TEMPO:
					assertWarning(trackNumber == 1,
									"Tempo changes must be placed in track 1");
					value = getFixLength(3);
					current = new event(META_EVENT, intToTime(time), SET_TEMPO,
										value, time, trackNumber, 0);
					addEvent(current);
					tempoMap->add(current);
					setTempo(current);
					break;
				case SEQUENCE_NUMBER:
					value = getFixLength(length);
					addEvent(new event(META_EVENT, intToTime(time),
								SEQUENCE_NUMBER, value, 0, trackNumber, 0));
					break;
				case KEY_SIGNATURE:
					value = getByte();
					note = getByte();
					addEvent(new event(META_EVENT, intToTime(time),
								KEY_SIGNATURE, value, note, trackNumber, 0));
					break;
				case TIME_SIGNATURE:
					note = getByte();
					velocity = getByte();
					value = getByte();
					channel = getByte();
					current = new event(META_EVENT, intToTime(time),
							TIME_SIGNATURE, note, velocity, trackNumber, value);
					current->flags = channel;
					addEvent(current);
					break;
				case SMPTE_OFFSET:
					note = getByte();
					velocity = getByte();
					value = getByte();
					current = new event(META_EVENT, intToTime(time),
							SMPTE_OFFSET, note, velocity, trackNumber, value);
					current->flags = getByte();
					current->salience = getByte(); // ran out of ints
					addEvent(current);
					break;
				case SEQUENCER_SPECIFIC:
					note = getByte();
					current = new event(META_EVENT, intToTime(time),
							SEQUENCER_SPECIFIC, note, 0, trackNumber, 0);
					getData(length - 1, current);
					addEvent(current);
					break;
				case END_OF_TRACK:
					assertWarning(byteCount == trackLength, "Unexpected EOTrk");
					break;
				case TEXT_EVENT:
				case COPYRIGHT_NOTICE:
				case SEQUENCE_NAME:
				case INSTRUMENT_NAME:
				case LYRIC:
				case MARKER:
				case CUE_POINT:
				case CHANNEL_PREFIX:
					current = new event(META_EVENT, intToTime(time),
								command, 0, 0, trackNumber, 0);
					getData(length, current);
					addEvent(current);
					break;
				default:
					assertWarning(false, "Unknown Meta event in MIDI data");
					in.ignore(length);
					byteCount += length;
					break;
				}
			}
			break;
		default:
			throw Exception("Unknown event in MIDI data");
		}
	}
	for (int c = 0; c < 16; c++)
		for (int i = 0; i < 128; i++)
			assertWarning(currentNote[c][i] == NULL, "NoteOn without NoteOff");
} // readTrack()
	
// Class for input of TMF format data

iTMF :: iTMF(const char* nm, eventList* e): InputMIDIBase(nm, e) {
	if (!matchString(in, "MFile "))
		throw Exception("Parse error: no TMF header");
	in >> format >> trackCount >> midiDivision >> ws;
	tempoMap = new eventList();
	warned = false;
	for (int trackNumber = 1; trackNumber <= trackCount; trackNumber++) {
		readTrack(trackNumber);
	}
	if (!listEvents)
		listEvents = new eventList();
	if (events) {
		events->addContents(listEvents);
		// listEvents->applyPedalEvents();
	}
} // iTMF constructor

// Converts MIDI clock cycles to real time; allows for tempo changes
double InputMIDIBase :: intToTime(int midiTime) {
	if (!assertWarning(warned || (tempoMap->next->ev!=NULL) || (midiTime == 0),
						"Tempo undefined: using 120MM"))
		warned = true;
	for (const eventList* e = tempoMap->next; e->hasMore(); e = e->next)
		if ((clockOffset <= e->ev->voice) && (e->ev->voice < midiTime))
			setTempo(e->ev);
	return timeOffset + double(midiTime - clockOffset) / clockFactor;
} // intToTime()

void InputMIDIBase :: resetClock() {
	clockOffset = 0;
	timeOffset = 0;
	clockFactor = 2.0 * double(midiDivision);
}

void InputMIDIBase :: setTempo(event* tempoEvent) {
	timeOffset += double(tempoEvent->voice - clockOffset) / clockFactor;
	clockOffset = tempoEvent->voice;
	clockFactor = 1000000.0 * double(midiDivision)/double(tempoEvent->volume);
}

void InputMIDIBase :: addEvent(event* e) {
	if (events == NULL)
		events = new eventTree(e);
	else
		events = (eventTree*) events->add(e);
} // addEvent()

bool iTMF :: addMetaEvent(int midiTime, int trackNumber) {
	string line;
	getline(in, line);
	int stop = line.find(' ');
	if (stop < 0)
		stop = line.size();
	char command[stop+1];
	strcpy(command, line.substr(0, stop).c_str());
	event* e = new event(META_EVENT, intToTime(midiTime),0,0,0,trackNumber,0);
	if (isText(command)) {
		e->pitch = TEXT_EVENT;
	} else if (isCopyright(command)) {
		e->pitch = COPYRIGHT_NOTICE;
	} else if (isSeqName(command)) {
		e->pitch = SEQUENCE_NAME;
	} else if (isTrkName(command)) {
		e->pitch = CHANNEL_PREFIX;
	} else if (isInstrName(command)) {
		e->pitch = INSTRUMENT_NAME;
	} else if (isLyric(command)) {
		e->pitch = LYRIC;
	} else if (isMarker(command)) {
		e->pitch = MARKER;
	} else if (isCue(command)) {
		e->pitch = CUE_POINT;
	} else if (isTrackEnd(command)) {
		delete e;
		return true;
	} else if (isMetaOther(command)) {
		if (isalnum(command[3]))	// check for single digit numbers
			e->pitch = hextoi(command[2]) << 4 + hextoi(command[3]);
		else
			e->pitch = hextoi(command[2]);
	} else {
		if (parameters->debug("tmf", "iTMF"))
			cerr << "Unrecognised meta event: " << command << "\n";
		return false;
	}
	return false;
} // iTMF::addMetaEvent()

/*
void iTMF :: applyPedals() {
	event* damper = 0;
	event* soft = 0;
	double damperOnTime = softOnTime = -1.0;
	// ***************
	int value; double time;

			switch (note) {
				case DAMPER_PEDAL:
					if (isPedalOn(value) != isPedalOn(damper)) {//state change
						if (parameters->debug("tmf", "pedal"))
							cout << "Damper pedal "
								 << (isPedalOn(value)? "on": "off")
								 << " at t = " << setprecision(3)
								 << intToTime(time) << "\n";
						if (isPedalOn(value)) {
							assertWarning(damperOnTime < 0,
								"Parse error in tmf data: DamperOn");
							damperOnTime = intToTime(time);
						} else {
							assertWarning(damperOnTime >= 0,
								"Parse error in tmf data: DamperOff");
							pedalEvents->add(new event(damperOnTime,
								intToTime(time), DAMPER_PEDAL));
							damperOnTime = -1.0;
						}
					}
					damper = value;
					break;
				case SOFT_PEDAL:
					if (isPedalOn(value) != isPedalOn(soft)) { // state change
						if (parameters->debug("tmf", "pedal"))
							cout << "Soft pedal "
								 << (isPedalOn(value)? "on": "off")
								 << " at t = " << setprecision(3)
								 << intToTime(time) << "\n";
					}
					soft = value;
					break;
			}
			if (parameters->debug("tmf", "pedal"))
				cout << "Damper = " << damper << "; Soft = " << soft <<"\n";
}
*/

int iTMF :: getNoteNumber(char c) {
	int value = 0;
	switch(toupper(c)) {
		case 'A': value =  9; break;
		case 'B': value = 11; break;
		case 'C': value =  0; break;
		case 'D': value =  2; break;
		case 'E': value =  4; break;
		case 'F': value =  5; break;
		case 'G': value =  7; break;
		default: assertWarning(false, "Invalid note name");
	}
	return value;
} // getNoteNumber()

int iTMF :: getVal(char* s) {
	int c, value = -1;
	if (s != NULL) {
		// matches any substring without rewinding
		if (matchString(in >> ws, s, false) && (strcmp(s, "note") == 0)) {
			// only expects text for "note=...", not for "n="
			if (matchString(in, "=") && (in >> value >> ws)) {
				value = getNoteNumber(in.get());
				while(TRUE) {
					c = in.get();
					if ((c == '#') || (c == '+'))
						value++;
					else if ((c == 'b') || (c == '-'))
						value--;
					else
						break;
				}
				if (c == '1') { // just in case octave == 10
					if (isdigit(c = in.get()))
						c += 10;
					else
						c = 1;
				}
				value += (c - '0') * 12;
				in >> ws;
			}
		} else if (!assertWarning(matchString(in, "=") && (in >> value >> ws),
									"bad scan in MIDI data"))
			value = 0;
		if (!assertWarning((value >= 0) && (value <= 127),
							"bad value in MIDI data"))
			value = 0;
	} else if (!assertWarning(bool(in >> value >> ws), "bad scan in MIDI data"))
		value = 0;
	return value;
} // getVal()

int iTMF :: getChannel() { 
	int v = getVal("ch") - 1;	// mf2t/t2mf counts from 1
	if (!assertWarning((v >= 0) && (v < 16), "bad channel number in MIDI data"))
		return 0;
	return v;
} // getChannel()

void iTMF :: getData(event* e) {
	in >> ws;
	if (in.peek() == '"')
		getString(e);
	else
		getHex(e);
} // getData()

void iTMF :: getString(event* e) {
	string buf;
	char c, d1, d2;
	in.ignore();	// opening "
	while(true) {
		in >> c;	// skip ws ??
		if (c == '\\') {
			in >> c;
			switch(c) {
				case '0':
					c = 0;
				case '\\':
				case '"':
					break;
				case 'r':
					c = '\r';
					break;
				case 'n':
					c = '\n';
					break;
				case 'x':
					in >> d1 >> d2;
					c = hextoi(d1) << 4 + hextoi(d2);
					break;
				case '\n':
					continue;
			}
		} else if (c == '"') {
			if (e != NULL) {
				e->flags = buf.length();
				e->data = new char[e->flags+1];
				buf.copy(e->data, e->flags);
				e->data[e->flags] = 0;
				return;
			}
		} else if (c == '\n')
			throw Exception("Unterminated string in TMF file");
		buf += c;
	}
} // getString()

void iTMF :: getHex(event* e) {
	char c, d1, d2;
	string buf;
	while (true) {
		if (in.peek() != '\n')
			break;
		if (isspace(in.peek()))
			in.ignore();
		else if (in.peek() == '\\')
			in.ignore(2);
		else {
			in >> ws >> d1 >> d2;
			c = hextoi(d1) << 4 + hextoi(d2);
			buf += c;
		}
	}
	if (e != NULL) {
		e->flags = buf.length();
		e->data = new char[e->flags+1];
		buf.copy(e->data, e->flags);
		e->data[e->flags] = 0;
	}
} // getHex()

// old getData() ...
//		getline(in, buf);
//		int last = buf.size() - 1; 
//		if ((last < 0) || (buf.at(last) != '\\'))
//			break;
//		all += buf.substr(0, last);
//	}
//	all += buf;
//	if (e != NULL) {
//		e->flags = all.length();
//		e->data = new char[e->flags];
//		all.copy(e->data, e->flags);
//	}

void iTMF :: readTrack(int trackNumber) {
	int time;
	int channel, note, velocity, value;
	event* current;
	const int SZ = 256;
	char eventString[SZ];
	if (!matchString(in, "MTrk\n"))
		throw Exception("Parse error: no MTrk");
	event* currentNote[16][128] = { { NULL } };
	int voice[16] = { -1 };
	resetClock();
	while (in >> time >> ws) {
		in.getline(eventString, SZ, ' ');
		if (parameters->debug("tmf", "iTMF"))
			cerr << "Reading: " << time << " " << eventString << "\n";
		if (isNoteOn(eventString)) {
			channel = getChannel();
			note = getNote();
			velocity = getVelocity();
			if (currentNote[channel][note] != NULL) {
				if (velocity == 0) {	// old style note off
					currentNote[channel][note]->offset =
						currentNote[channel][note]->noteOff = intToTime(time);
					currentNote[channel][note] = NULL;
				} else
					assertWarning(false, "Double NoteOn in MIDI data");
			} else {
				current = new event(NOTE_ON, intToTime(time), note, velocity,
									voice[channel], trackNumber, channel);
				addEvent(current);
				currentNote[channel][note] = current; 
			}
		} else if (isNoteOff(eventString)) {
			channel = getChannel();
			note = getNote();
			getVelocity();
			if (currentNote[channel][note] == NULL)
				assertWarning(false, "NoteOff without NoteOn");
			else {
				currentNote[channel][note]->offset = intToTime(time);
				currentNote[channel][note]->noteOff = intToTime(time);
				currentNote[channel][note] = NULL;
			}
		} else if (isKeyAfterTouch(eventString)) {
			channel = getChannel();
			note = getNote();
			value = getValue();
			addEvent(new event(KEY_AFTERTOUCH, intToTime(time), note, value,
								voice[channel], trackNumber, channel));
		} else if (isChannelAfterTouch(eventString)) {
			channel = getChannel();
			value = getValue();
			addEvent(new event(CHANNEL_AFTERTOUCH, intToTime(time), value,value,
								voice[channel], trackNumber, channel));
		} else if (isControllerChange(eventString)) {	// "Par"
			channel = getChannel();
			note = getController();
			value = getValue();
			addEvent(new event(CONTROLLER_CHANGE, intToTime(time), note, value,
					 voice[channel],trackNumber,channel));
		} else if (isPitchBend(eventString)) {
			channel = getChannel();
			value = getValue();
			addEvent(new event(PITCH_WHEEL, intToTime(time), value, value,
								voice[channel], trackNumber, channel));
		} else if (isProgramChange(eventString)) {
			channel = getChannel();
			value = getProgram();
			voice[channel] = value;
			addEvent(new event(PROGRAM_CHANGE, intToTime(time), value, value,
								value, trackNumber, channel));
		} else if (isSysex(eventString) || isArbitrary(eventString)) {
			current = new event(SYSTEM_EXCLUSIVE, intToTime(time), 0,0,0,0,0);
			getData(current);
			addEvent(current);
		} else if (isTempo(eventString)) {
			assertWarning(trackNumber == 1,
				"Tempo changes must be placed in track 1");
			value = getVal(NULL);
			current = new event(META_EVENT, intToTime(time), SET_TEMPO,
								value, time, trackNumber, 0);
			addEvent(current);
			tempoMap->add(current);
			setTempo(current);
		} else if (isSeqNum(eventString)) {
			in >> value;
			addEvent(new event(META_EVENT, intToTime(time), SEQUENCE_NUMBER,
						value, 0, trackNumber, 0));
		} else if (isKeySig(eventString)) {
			in >> value >> ws;
			note = (matchString(in, "minor")? 0: 1);
			addEvent(new event(META_EVENT, intToTime(time), KEY_SIGNATURE,
						value, note, trackNumber, 0));
			in.ignore(SZ, '\n');
		} else if (isTimeSig(eventString)) {
			in >> note >> ws;
			in.ignore();	// the '/' between the first 2 arguments
			in >> velocity >> value >> channel;
			current = new event(META_EVENT, intToTime(time), TIME_SIGNATURE,
						note, velocity, trackNumber, value);
			current->flags = channel;
			addEvent(current);
		} else if (isSMPTE(eventString)) {
			in >> note >> velocity >> value;
			current = new event(META_EVENT, intToTime(time), SMPTE_OFFSET,
						note, velocity, trackNumber, value);
			in >> note >> velocity;
			current->flags = note;
			current->salience = velocity; // ran out of ints
			addEvent(current);
		} else if (isSeqSpec(eventString)) {
			in >> note;
			current = new event(META_EVENT, intToTime(time), SEQUENCER_SPECIFIC,
						note, 0, trackNumber, 0);
			getData(current);
			addEvent(current);
		} else if (isMeta(eventString)) {
			if (addMetaEvent(time, trackNumber))  // returns (metaEvt == trkEnd)
				break;
		} else {
			if (parameters->debug("tmf", "iTMF"))
				cerr << "Warning: Command not recognised: "<<eventString<<"\n";
			getData(NULL);
		}
	}
	if (!matchString(in, "TrkEnd\n"))
		throw Exception("Parse error: no TrkEnd");
	for (int c = 0; c < 16; c++)
		for (int i = 0; i < 128; i++)
			assertWarning(currentNote[c][i] == NULL, "NoteOn without NoteOff");
} // readTrack()
	
// Base class for output of MIDI and TMF format data

OutputMIDIBase ::
OutputMIDIBase(const char* nm, eventList* e, double st, double ln):
		fileName(nm), out(nm), events(e), start(st), length(ln), head(-1),
		previousEvent(0), clockOffset(0), timeOffset(0), timeFactor(1000.0) {
	if (!out.is_open())
		throw Exception("Unable to open output file");
	trackCount = e->countTracks();
	format = (trackCount == 1)? 0: 1;
	midiDivision = 480; // overridden by default Boesendorfer timing
	midiTempo = 480000;
	if (!strcmp(parameters->getString("boesendorferTiming","on"),"on")){
		midiDivision = 400;
		midiTempo = 500000;
	}
	midiDivision = parameters->getInt("midiDivision", midiDivision);
	midiTempo = parameters->getInt("midiTempo", midiTempo);
} // OutputMIDIBase constructor

// Converts real time to MIDI clock cycles; allows for tempo changes
int OutputMIDIBase :: timeToInt(double time) {
    return clockOffset + (int) rint((time - timeOffset) * timeFactor);
} // timeToInt()

void OutputMIDIBase :: resetClock() {
	clockOffset = 0;
	timeOffset = 0;
	setTempo(0, midiTempo);
} // resetClock()

void OutputMIDIBase :: setTempo(double time, int newTempo) {
	clockOffset += (int) rint((time - timeOffset) * timeFactor);
	timeOffset = time;
	timeFactor = 1000000.0 * double(midiDivision) / double(newTempo);
} // setTempo()

void OutputMIDIBase :: writeTrack(int trackNumber) {
	int voices[16] = {-1};
	event* damper = NULL;
	event* soft = NULL;
	bool started = false;
	resetClock();
	previousEvent = 0;
	head = -1;
	if (events->next->ev == NULL)
		return;
	startTrack(trackNumber, events->next->isTempoChange());
	for (eventList* e = events->next; e->hasMore(); e = e->next) {
		if (e->isTempoChange()) {
			setTempo(e->ev->onset, e->ev->volume);
			if (trackNumber == 1)
				putEvent(e->ev);
			continue;
		}
		if (e->ev->track != trackNumber)
			continue;
		if ((e->ev->onset < start) ||
				((length >= 0) && (e->ev->onset > start + length))) {
			if (e->isController()) {
				if (e->ev->pitch == DAMPER_PEDAL)
					damper = e->ev;
				else if (e->ev->pitch == SOFT_PEDAL)
					soft = e->ev;
			}
		} else {
			if (!started) {
				started = true;
				if (damper != NULL) {
					double tmp = damper->onset;
					damper->onset = e->ev->onset;
					putEvent(damper);
					damper->onset = tmp;
				}
				if (soft != NULL) {
					double tmp = soft->onset;
					soft->onset = e->ev->onset;
					putEvent(soft);
					soft->onset = tmp;
				}
			}
			if (e->isNote() && (e->ev->voice != -1) && 
					(e->ev->voice != voices[e->ev->channel])) {
				e->ev->eventType = PROGRAM_CHANGE;
				putEvent(e->ev);
				e->ev->eventType = NOTE_ON;
				voices[e->ev->channel] = e->ev->voice;
			}
			putEvent(e->ev);
		}
	} // for each event
	endTrack();
} // writeTrack()


// Class for output of TMF format

oTMF :: oTMF(const char* nm, eventList* e, double st, double ln):
		OutputMIDIBase(nm, e, st, ln) {
	out << "MFile " << format << " " << trackCount << " " << midiDivision<<"\n";
	for (int trackNumber = 1; trackNumber <= trackCount; trackNumber++)
		writeTrack(trackNumber);
} // oTMF constructor

// Outputs all note off messages occurring before 'time'
void oTMF :: flushQueue(double time) {
    while ((head >= 0) && (eventQ[head]->noteOff <= time)) {
		out << setw(6) << timeToInt(eventQ[head]->noteOff)
			<< " Off  ch=" << setw(3) << eventQ[head]->channel+1
			<< " n=" << setw(3) << eventQ[head]->pitch
			<< " v=" << setw(3) << eventQ[head]->volume << "\n";
		head--;
	}
} // oTMF::flushQueue()

void oTMF :: startTrack(int trackNumber, bool hasTempo) {
	out << "MTrk\n";
	if (trackNumber == 1) {
		if (!hasTempo)
			out << "     0 Tempo " << midiTempo << "\n";
		char* defaults = parameters->getString("tmfDefaults");
		if (defaults) {
			if (strlen(defaults))
				out << "     0 " << defaults << "\n";
		} else if (!strcmp(parameters->getString("stereo","off"), "on")) {
			// Pan piano and metronome to different channels
			out << "     0 Par  ch=  1 c= 10 v=  0\n"
				   "     0 Par  ch= 10 c= 10 v=127\n";
		}
	}
} // oTMF::startTrack()

void oTMF :: endTrack() {	// output remaining noteOffs & text/midi postamble
	double end = (head >= 0)? eventQ[0]->noteOff: 0;
	if (previousEvent > end)
		end = previousEvent;
	flushQueue(end); // flush all
	out << setw(6) << timeToInt(end) << " Meta TrkEnd\nTrkEnd" << endl;
} // oTMF::endTrack()

// Outputs an event to the TMF file; noteOffs are queued in case
//  intervening events are still to come; queued events are output as necessary
void oTMF :: putEvent(const event* ev) {
	if (ev->eventType != NOTE_OFF) {
		assertWarning(ev->onset >= previousEvent, "MIDI event out of sequence");
		previousEvent = ev->onset;
		flushQueue(ev->onset);
		out << setw(6) << timeToInt(ev->onset);
	}
	switch (ev->eventType) {
		case NOTE_ON:
			out << " On   ch=" << setw(3) << ev->channel+1
				<< " n=" << setw(3) << ev->pitch
				<< " v=" << setw(3) << ev->volume << "\n";
			// no break here - need to queue the NOTE_OFF event 
		case NOTE_OFF:		// normally only falls through from NOTE_ON
			if (++head >= MIDI_QUEUE_SIZE)
				throw Exception("Internal overflow in putEvent()");
			else {
				int i = head;
				for ( ; (i > 0) && (ev->noteOff >= eventQ[i-1]->noteOff); i--)
					eventQ[i] = eventQ[i-1];
				eventQ[i] = ev;
			}
			break;
		case KEY_AFTERTOUCH:
			out << " PoPr ch=" << setw(3) << ev->channel+1
				<< " n=" << setw(3) << ev->pitch
				<< " v=" << setw(3) << ev->volume << "\n";
			break;
		case CONTROLLER_CHANGE:	// parameter change
			out << " Par  ch=" << setw(3) << ev->channel+1
				<< " c=" << setw(3) << ev->pitch
				<< " v=" << setw(3) << ev->volume << "\n";
			break;
		case PROGRAM_CHANGE:
			out << " PrCh ch=" << setw(3) << ev->channel+1
				<< " p=" << setw(3) << ev->voice << "\n";
			break;
		case CHANNEL_AFTERTOUCH:
			out << " ChPr ch =" << setw(3) << ev->channel+1
				<< " v=" << setw(3) << ev->volume << "\n";
			break;
		case PITCH_WHEEL:
			out << " Pb   ch=" << setw(3) << ev->channel+1
				<< " v=" << setw(3) << ev->volume << "\n";
			break;
		case SYSTEM_EXCLUSIVE:
			out << " SysEx " << ev->data << "\n";
			break;
		case META_EVENT:
			switch(ev->pitch) {
				case SET_TEMPO:
					out << " Tempo " << ev->volume << "\n";
					break;
				default:
					// assertWarning(false, "Unknown Meta Event");
					if (ev->data)
						out << " Meta Text \"" << ev->data << "\"\n";
					else
						out << " Meta Text \"\"\n";
					break;
				} // inner switch
			break;
		default:        // Error: should not reach here
			throw Exception("Invalid MidiMessage in putEvent()");
	} // outer switch
} // oTMF::putEvent()


// Class for MIDI output

oMIDI :: oMIDI(const char* nm, eventList* e, double st, double ln):
        OutputMIDIBase(nm, e, st, ln) {
	out.write("MThd", 4);
	write32BE(6, out);
	write16BE(format, out);
	write16BE(trackCount, out);
	write16BE(midiDivision, out);
	for (int trackNumber = 1; trackNumber <= trackCount; trackNumber++)
		writeTrack(trackNumber);
} // oMIDI constructor

int oMIDI :: deltaTime(int midiTime) {
	int tmp = midiTime - deltaMarker;
	deltaMarker = midiTime;
	return tmp;
}

void oMIDI :: putVlen(int value) {
	unsigned char buf[5];
	int count = 4;
	while (value > 0x7F) {
		buf[count--] = 0x80 | (value & 0x7F);
		value >>= 7;
	}
	buf[count] = 0x80 | (value & 0x7F);
	buf[4] &= 0x7F;
	out.write(buf+count, 5-count);
}

// Outputs all note off messages occurring before 'time'
void oMIDI :: flushQueue(double time) {
    while ((head >= 0) && (eventQ[head]->noteOff <= time)) {
		putVlen(deltaTime(timeToInt(eventQ[head]->noteOff)));
		unsigned char buf[3];
		buf[0] = NOTE_OFF | eventQ[head]->channel;
		buf[1] = eventQ[head]->pitch;
		buf[2] = eventQ[head]->volume;
		out.write(buf, 3);
		head--;
	}
} // oMIDI::flushQueue()

void oMIDI :: startTrack(int trackNumber, bool hasTempo) {
	deltaMarker = 0;
	out.write("MTrk", 4);
	lengthPointer = out.tellp();
	out.write("XXXX", 4);
	if (trackNumber == 1) {
		if (!hasTempo) {
			putVlen(0);
			writeHexString("FF5103", out);
			write24BE(midiTempo, out);
		}
		char* defaults = parameters->getString("midiDefaults");
		if (defaults) {
			if (strlen(defaults)) {
				putVlen(0);
				writeHexString(defaults, out);
			}
		} else if (!strcmp(parameters->getString("stereo","off"), "on")) {
			// Pan piano and metronome to different channels
			putVlen(0);
			writeHexString("B00A00", out);		// 0 Par  ch=  1 c= 10 v=  0
			putVlen(0);
			writeHexString("B90A7F", out);		// 0 Par  ch= 10 c= 10 v=127
		}
	}
} // oMIDI::startTrack()

void oMIDI :: endTrack() {	// output remaining noteOffs & text/midi postamble
	if (head >= 0)
		flushQueue(eventQ[0]->noteOff); // flush all
	putVlen(0);
	writeHexString("FF2F00", out);
	int trackLength = out.tellp() - lengthPointer - 4;
	out.seekp(lengthPointer);
	write32BE(trackLength, out);
	out.seekp(0, ios::end);
} // oMIDI::endTrack()

// Outputs an event to the MIDI file; noteOffs are queued in case
//  intervening events are still to come; queued events are output as necessary
void oMIDI :: putEvent(const event* ev) {
	unsigned char buf[3];
	if (ev->eventType != NOTE_OFF) {
		assertWarning(ev->onset >= previousEvent, "MIDI event out of sequence");
		previousEvent = ev->onset;
		flushQueue(ev->onset);
		putVlen(deltaTime(timeToInt(ev->onset)));
		buf[0] = ev->eventType | ev->channel;
		buf[1] = ev->pitch;
		buf[2] = ev->volume;
	}
	switch (ev->eventType) {
		case NOTE_ON:
			out.write(buf, 3);
			// no break here - need to queue the NOTE_OFF event 
		case NOTE_OFF:		// normally only falls through from NOTE_ON
			if (++head >= MIDI_QUEUE_SIZE)
				throw Exception("Internal overflow in putEvent()");
			else {
				int i = head;
				for ( ; (i > 0) && (ev->noteOff >= eventQ[i-1]->noteOff); i--)
					eventQ[i] = eventQ[i-1];
				eventQ[i] = ev;
			}
			break;
		case KEY_AFTERTOUCH:
		case CONTROLLER_CHANGE:	// parameter change
			out.write(buf, 3);
			break;
		case PROGRAM_CHANGE:
			buf[1] = ev->voice;
			out.write(buf, 2);
			break;
		case CHANNEL_AFTERTOUCH:
		case PITCH_WHEEL:
			buf[1] = ev->volume;
			out.write(buf, 2);
			break;
		case SYSTEM_EXCLUSIVE:
			writeHexString("F0", out);
			putVlen(ev->flags);
			out.write(ev->data, ev->flags);
			break;
		case META_EVENT:
			switch(ev->pitch) {
				case SEQUENCE_NUMBER:
					writeHexString("FF0002", out);
					write16BE(ev->volume , out);
					break;
				case TEXT_EVENT:
				case COPYRIGHT_NOTICE:
				case SEQUENCE_NAME:
				case INSTRUMENT_NAME:
				case LYRIC:
				case MARKER:
				case CUE_POINT:
				case CHANNEL_PREFIX:
					write8(META_EVENT, out);
					write8(ev->pitch, out);
					putVlen(ev->flags);
					out.write(ev->data, ev->flags);
					break;
				case END_OF_TRACK:
					writeHexString("FF2F00", out);
					break;
				case SET_TEMPO:
					writeHexString("FF5103", out);
					write24BE(ev->volume, out);
					break;
				case SMPTE_OFFSET:
					writeHexString("FF5405", out);
					write8(ev->volume, out);
					write8(ev->voice, out);
					write8(ev->channel, out);
					write8(ev->flags, out);
					write8((int)ev->salience, out);
					break;
				case TIME_SIGNATURE:
					writeHexString("FF5804", out);
					write8(ev->volume, out);
					write8(ev->voice, out);
					write8(ev->channel, out);
					write8(ev->flags, out);
					break;
				case KEY_SIGNATURE:
					writeHexString("FF5902", out);
					write8(ev->volume, out);
					write8(ev->voice, out);
					break;
				case SEQUENCER_SPECIFIC:
					writeHexString("FF7F", out);
					putVlen(ev->flags+1);
					write8(ev->volume, out);
					out.write(ev->data, ev->flags);
					break;
				default:
					assertWarning(false, "Unknown Meta Event");
					writeHexString("FF01", out);
					if (ev->data) {
						putVlen(ev->flags);
						out.write(ev->data, ev->flags);
					} else
						putVlen(0);
					break;
				} // inner switch
			break;
		default:        // Error: should not reach here
			throw Exception("Invalid MidiMessage in putEvent()");
	} // outer switch
} // oMIDI::putEvent()
