//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: eventMidi.cpp
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
#include "param.h"
#include "tmf.h"
#include "tree.h"
#include "util.h"

// #define M2T "/raid/music/bin/mf2t"

// MIDI codes for damper and soft pedals
#define DAMPER_PEDAL 64
#define SOSTENUTO_PEDAL 66
#define SOFT_PEDAL 67

// for distinguishing pedal "on" (0x40-0x7F) from pedal "off" (0x00-0x3F)
#define PEDAL_ON(x) ((x) >= 0x40)

// Methods used in parsing MIDI text files
//  NOTE: readTMF() is deprecated; replaced by class iTMF

int isNoteOn(char* x) {
    return (strcasecmp((x),"on") == 0);
}

int isNoteOff(char* x) {
    return (strcasecmp((x),"off") == 0);
}

int isPolyPressure(char* x) {
    return ((strcasecmp((x),"popr") == 0) || (strcasecmp((x),"polypr") == 0));
}

int isChannelPressure(char* x) {
    return ((strcasecmp((x),"chpr") == 0) || (strcasecmp((x),"chanpr") == 0));
}

int isParameter(char* x) {
	return ((strcasecmp((x),"par") == 0) || (strcasecmp((x),"param") == 0));
}

int isPitchBend(char* x) {
    return (strcasecmp((x),"pb") == 0);
}

int isProgramChange(char* x) {
    return ((strcasecmp((x),"prch") == 0) || (strcasecmp((x),"progch") == 0));
}

int isSysex(char* x) {
	return (strcasecmp((x),"sysex") == 0);
}

int isArbitrary(char* x) {
	return (strcasecmp((x),"arb") == 0);
}

int isTempo(char* x) {
	return (strcasecmp((x),"tempo") == 0);
}

int isMeta(char* x) {
	return (strcasecmp((x),"meta") == 0);
}

int isTrackEnd(char* x) {
	return (strcasecmp((x),"trkend") == 0);
}

int getNoteNumber(char c) {
	int value = 0;
	switch(toupper(c)) {
		case 'A': value =  9; break;
		case 'B': value = 11; break;
		case 'C': value =  0; break;
		case 'D': value =  2; break;
		case 'E': value =  4; break;
		case 'F': value =  5; break;
		case 'G': value =  7; break;
		default: assertWarning(FALSE, "Invalid note name");
	}
	return value;
} // getNoteNumber()

int getVal(istream& f, char* s) {
	int c, value = -1;
	if (s != NULL) {
		// matches any substring without rewinding
		if (matchString(f, s, false) && (strcmp(s, "note") == 0)) {
			// only expects text for "note=...", not for "n="
			if (matchString(f, "=") && (f >> value >> ws)) {
				value = getNoteNumber(f.get());
				while(TRUE) {
					c = f.get();
					if ((c == '#') || (c == '+'))
						value++;
					else if ((c == 'b') || (c == '-'))
						value--;
					else
						break;
				}
				if (c == '1') { // just in case octave == 10
					if (isdigit(c = f.get()))
						c += 10;
					else
						c = 1;
				}
				value += (c - '0') * 12;
				f >> ws;
			}
		} else if (!assertWarning(matchString(f, "=") && (f >> value >> ws),
									"bad scan in MIDI data"))
			value = 0;
		if (!assertWarning((value >= 0) && (value <= 127),
							"bad value in MIDI data"))
			value = 0;
	} else if (!assertWarning(bool(f >> value >> ws), "bad scan in MIDI data"))
		value = 0;
	return value;
} // getVal()

int getChannel(istream& f) { 
	int v = getVal(f, "ch") - 1;	// mf2t/t2mf counts from 1
	if (!assertWarning((v >= 0) && (v < 16), "bad channel number in MIDI data"))
		return 0;
	return v;
} // getChannel()

int getNote(istream& f)       { return getVal(f, "note"); }
int getVelocity(istream& f)   { return getVal(f, "vol");  }
int getValue(istream& f)      { return getVal(f, "val");  }
int getController(istream& f) { return getVal(f, "con");  }
int getProgram(istream& f)    { return getVal(f, "prog"); }

// Converts MIDI clock cycles to real time; allows for tempo changes
double intToTime(int clocks, double newTempoFactor = -1) {
	static double factor = 1000.0;
	static int clockOffset = 0;
	static double timeOffset = 0;
	if (newTempoFactor > 0) {
		if (clocks > 0) {
			timeOffset += double(clocks - clockOffset) / factor;
			clockOffset = clocks;
		} else {	// reinitialise in case it is a new file
			clockOffset = 0;
			timeOffset = 0;
		}
		factor = newTempoFactor;
	}
	return timeOffset + double(clocks - clockOffset) / factor;
} // intToTime()

// Adjusts note lengths to correspond to the use of the pedal
void eventList :: combinePedalEvents(eventList* pedals) {
	if (parameters->debug("eventMidi", "pedal")) {
		cout << "PEDAL:\n";
		pedals->print();
		cout << "NOTES BEFORE:\n";
		print();
	}
	eventList* current = next;
	for (pedals = pedals->next; pedals->ev != NULL; pedals = pedals->next) {
		if (pedals->ev->pitch != DAMPER_PEDAL)
			continue;
		double on = pedals->ev->onset;
		double off = pedals->ev->offset;
		while ((current->ev != NULL) && (current->ev->onset < off))
			current = current->next;
		current = current->prev;
		while ((current->ev != NULL) && (current->ev->onset > off))
			current = current->prev;
		while ((current->ev != NULL) && (current->ev->onset > on - 10.0)) {
			// assume notes sustain max 10s (for pedal to be useful)
			// check each note in this range
			if (!current->ev->isMidiMessage() && (current->ev->offset > on) &&
						(current->ev->offset < off)) {
				current->ev->offset = off;
				if (parameters->debug("eventMidi", "pedal"))
					current->ev->print();
			}
			current = current->prev;
		}
	}
	event* prevNote[128][16];		// correct for overlap
	for (int i=0; i < 128; i++)
		for (int j=0; j < 16; j++)
			prevNote[i][j] = NULL;
	event* e;
	for (current=nextNote(); current->ev != NULL; current=current->nextNote()) {
		if ((e = prevNote[current->ev->pitch][current->ev->channel]) != NULL)
			if (e->offset > current->ev->onset)
				e->offset = current->ev->onset;
		prevNote[current->ev->pitch][current->ev->channel] = current->ev;
	}
	if (parameters->debug("eventMidi", "pedal")) {
		cout << "NOTES AFTER:\n";
		print();
	}
} // combinePedalEvents()

// Creates an event list by reading from a MIDI binary (.mid), MIDI text (.tmf),
//  match (.match)  or  beat (.beat)  file
eventList :: eventList(const char* fileName, double start, double length) {
	init(NULL, this, this);
	char* formatString = parameters->getString("format");
	start = parameters->getDouble("start", start);
	length = parameters->getDouble("length", length);
	if (fileNameMatch(fileName, formatString, "tmf")) {
		// readTMF(fileName);
		try {
			iTMF readFile(fileName, this);
		} catch (Exception e) {
			e.print();
			throw;
		}
	} else if (fileNameMatch(fileName, formatString, "mid")) {
		// char buf[250];
		// char* m2t = parameters->getString("m2t", M2T);
		// strcpy(buf, m2t);
		// strcat(buf, " ");
		// strcat(buf, fileName);
		// strcat(buf, " ");
		// m2t = buf+strlen(buf);
		// makeFileName(fileName, NULL, ".tmf", buf+strlen(buf));
		// system(buf);
		// readTMF(m2t);
		try {
			iMIDI readFile(fileName, this);
		} catch (Exception e) {
			e.print();
			throw;
		}
	} else if (fileNameMatch(fileName, formatString, "match")) {
		try {
			readMatchFile(fileName);
			// combinePedalTrack(fileName);
		} catch (Exception& e) {
			e.print();
			throw;
		}
	} else if (fileNameMatch(fileName, formatString, "beat")) {
		readBeatFile(fileName);
	} else if (!assertWarning(FALSE,
				"Unknown file extension: use --format to disambiguate"))
		return;
	if (start > 0)
		clear(0, start, NOTE_ON);
	if (length >= 0)
		clear(start + length, -1, NOTE_ON);
	//	{
	//	eventList* end = nextEvent(start + length);
	//	if (end->ev != NULL) {
	//		assertWarning(FALSE, "Incomplete implementation");
	//		end->prev->next = this;
	//		end = new eventList(NULL, prev, end);
	//		prev = end->next->prev;
	//		end->next->prev = end;
	//		end->prev->next = end;
	//		delete(end);
	//	}
	//}
} // eventList(char*) constructor

// Parses MIDI text format data produced by mf2t 
//  NOTE: deprecated.  Use class iTMF.
void eventList :: readTMF(const char* fileName) {
	ifstream f(fileName);
	int format, trackCount, division, tempo = 0, time;
	int track, channel, note, velocity, damper, soft, value;
	double damperOnTime, softOnTime;
	eventList* pedalEvents;
	const int SZ = 80;
	char buf[SZ];
	char eventString[SZ];
	eventTree* events = NULL;
	event* currentNote[16][128];
	event* current;
	int voice[16];

	for (int c = 0; c < 16; c++) {
		voice[c] = -1;
		for (int i = 0; i < 128; i++)
			currentNote[c][i] = NULL;
	}
	pedalEvents = new eventList();
	if (!assertWarning(f.is_open(), "Could not open TMF file"))
		return;
	if (!assertWarning(matchString(f, "MFile "), "Parse error: no TMF header"))
		return;
	f >> format >> trackCount >> division;
	f.ignore();		// CR
	intToTime(0, double(2 * division)); // default 120MM = 2 beat/sec
	for (track = 0; track < trackCount; track++) { // read each track
		if (parameters->debug("eventMidi", "basic"))
			cout << "Reading track " << track << "\n";
		damper = soft = 0;
		damperOnTime = softOnTime = -1.0;
		if (!assertWarning(matchString(f, "MTrk\n"), "Parse error: no MTrk"))
			return;
		while (f >> time >> ws) {
			f.get(eventString, SZ, ' ');
			f.ignore();
			if (isNoteOn(eventString)) {
				channel = getChannel(f);
				note = getNote(f);
				velocity = getVelocity(f);
				if (currentNote[channel][note] != NULL) {
					if (velocity == 0) {	// old style note off
						currentNote[channel][note]->offset =
							currentNote[channel][note]->noteOff=intToTime(time);
						currentNote[channel][note] = NULL;
					} else
						assertWarning(FALSE, "Double NoteOn in MIDI data");
				} else {
					if (!assertWarning(tempo>0, "Tempo undefined: using 120MM"))
						tempo++;
					current = new event(NOTE_ON, intToTime(time), note,velocity,
										voice[channel], track, channel);
					if (events == NULL)
						events = new eventTree(current);
					else
						events = (eventTree*) events->add(current);
					currentNote[channel][note] = current; 
				}
			} else if (isNoteOff(eventString)) {
				channel = getChannel(f);
				note = getNote(f);
				getVelocity(f);
				if (currentNote[channel][note] == NULL)
					assertWarning(FALSE, "NoteOff without NoteOn");
				else
					currentNote[channel][note]->offset =
						currentNote[channel][note]->noteOff = intToTime(time);
				currentNote[channel][note] = NULL;
			} else if (isPolyPressure(eventString)) { // ignore these
				channel = getChannel(f);
				note = getNote(f);
				value = getValue(f);
			//	add(new event(intToTime(time)...... // need to check lots
			} else if (isChannelPressure(eventString)) {
				channel = getChannel(f);
				value = getValue(f);
			} else if (isParameter(eventString)) {
				channel = getChannel(f);
				note = getController(f);
				value = getValue(f);
				if (!assertWarning(tempo>0, "Tempo undefined: using 120MM"))
					tempo++;
				current = new event(NOTE_ON, intToTime(time), note, value,
									-1, track, channel);
				current->setMidiMessage();
				if (events == NULL)
					events = new eventTree(current);
				else
					events = (eventTree*) events->add(current);
				switch (note) {
					case DAMPER_PEDAL:
						if (PEDAL_ON(value) != PEDAL_ON(damper)) {//state change
							if (parameters->debug("eventMidi", "pedal"))
								cout << "Damper pedal "
									 << (PEDAL_ON(value)? "on": "off")
									 << " at t = " << setprecision(3)
									 << intToTime(time) << "\n";
							if (PEDAL_ON(value)) {
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
						if (PEDAL_ON(value) != PEDAL_ON(soft)) { // state change
							if (parameters->debug("eventMidi", "pedal"))
								cout << "Soft pedal "
									 << (PEDAL_ON(value)? "on": "off")
									 << " at t = " << setprecision(3)
									 << intToTime(time) << "\n";
						}
						soft = value;
						break;
					case SOSTENUTO_PEDAL:
						break;
					default:
						assertWarning(FALSE, "MIDI controller message ignored");
						break;
				}
				if (parameters->debug("eventMidi", "pedal"))
					cout << "Damper = " << damper << "; Soft = " << soft <<"\n";
			} else if (isPitchBend(eventString)) {
				getChannel(f);
				getValue(f);
			} else if (isProgramChange(eventString)) {
				channel = getChannel(f);
				voice[channel] = getProgram(f);
			} else if (isSysex(eventString) || isArbitrary(eventString)) {
				skipLine(f);
			} else if (isTempo(eventString)) {
				assertWarning((time == 0) || (track == 1),
					"Tempo change will not be processed correctly");
				tempo = getVal(f, NULL);
				intToTime(time, 1000000.0 * double(division) / double(tempo));
			} else if (isMeta(eventString)) {
				f.get(buf, SZ, '\n');
				skipLine(f);	// ignore Meta events (for now)
				if (isTrackEnd(buf))
					break;
				if (parameters->debug("eventMidi", "basic"))
					cout << "Unrecognised meta event: " << buf << "\n";
			} else {
				if (parameters->debug("eventMidi", "basic"))
					cout << "Command not recognised: " << eventString << "\n";
				skipLine(f);
			}
		}
		f.getline(buf, SZ);
		assertWarning(strcmp(buf, "TrkEnd") == 0, "Parse error: no TrkEnd");
	}
	if (parameters->debug("eventMidi", "basic"))
		cout << "Size=" << events->getSize()
			 << ", Height=" << events->getHeight() << "\n";
	for (int c = 0; c < 16; c++) {
		for (int i = 0; i < 128; i++)
			assertWarning(currentNote[c][i] == NULL, "Missing NoteOff");
	}
	events->addContents(this);
	// combinePedalEvents(pedalEvents);
	// writeMidiTextFile("tmp.tmf");
	// exit(0);
} // readTMF()

// Reads in a Prolog score+performance (.match) file; returns it as an eventList
// Lines in the match file can be of the form:
//		hammer_bounce-PlayedNote.
//		info(Attribute, Value).
//		insertion-PlayedNote.
//		ornament(Anchor)-PlayedNote.
//		ScoreNote-deletion.
//		ScoreNote-PlayedNote.
//		ScoreNote-trailing_score_note.
//		trailing_played_note-PlayedNote.
//		trill(Anchor)-PlayedNote.
// where ScoreNote is of the form
//		snote(Anchor,[NoteName,Modifier],Octave,Bar:Beat,Offset,Duration,
//				BeatNumber,DurationInBeats,ScoreAttributesList)
//		e.g. snote(n1,[b,b],5,1:1,0,3/16,0,0.75,[s])
// and PlayedNote is of the form
//		note(Number,[NoteName,Modifier],Octave,Onset,Offset,AdjOffset,Velocity)
//		e.g. note(1,[a,#],5,5054,6362,6768,53)
void eventList :: readMatchFile(const char* fileName) throw (Exception) {
	const int SZ = 250;
	char buf[SZ];
	char noteId[SZ];
	bool isBt;
	int eventFlags, numerator, denominator;
	char* element;
	char* endPtr;
	double versionNumber = 1.0;
	ifstream f(fileName);
	if (!f.is_open())
		throw Exception("Could not open match file");
	int timingCorrection =
			! strcmp(parameters->getString("timingCorrection", "off"), "on");
	int clockUnits = parameters->getInt("midiDivision", 480);//defaults for
	int clockRate = parameters->getInt("midiTempo", 500000); //matchfileVersion1
	element = parameters->getString("melody","norm");
	int noMelody = !strcmp(element, "off");
	int onlyMelody = !strcmp(element, "on");
	double beatLevel = parameters->getDouble("level");
	double onset, offset, eOffset, beat, displacement = -1.0;
	int velocity, pitch, octave;
	int linecount = 0;
	char c = f.peek();	// one character lookahead
	while (f && !f.eof()) {
		eventFlags = 0;
		if (parameters->debug("eventMidi", "basic"))
			cout << "Processing line " << ++linecount << endl;
		if (matchString(f, "info(")) {	// meta-data
			if (matchString(f, "timeSignature,")) {
				ostrstream ss1, ss2;
				f >> numerator;
				ss1 << "beatsPerBar=" << numerator << ends;
				parameters->add(ss1.str());
				skip(f, '/');
				f >> denominator;
				ss2 << "beatUnits=" << denominator;
				parameters->add(ss2.str());
				ss1.freeze(0);	// re-enable automatic deallocation
				ss2.freeze(0);
			} else if (matchString(f, "beatSubdivision,")) {
				strcpy(buf, "beatSubdivisions=");
				int i = strlen(buf);
				f.getline(buf+i, SZ-i, ']');
				strcat(buf, "]");
				parameters->add(buf);
			} else if (matchString(f, "matchFileVersion,")) {
				f >> versionNumber;
			} else if (matchString(f, "midiClockUnits,")) {
				f >> clockUnits;
			} else if (matchString(f, "midiClockRate,")) {
				f >> clockRate;
			}
			c = '%';	// don't expect the second half of the Prolog term
		} else if (matchString(f,"snote(")) {
			f.getline(noteId, SZ, ',');
			// skip(f, ',');	// identifier
			skip(f, ']');	// note name
			skip(f, ',');	// ',' after note name
			skip(f, ',');	// octave
			skip(f, ',');	// onset time (in beats, integer part, bar:beat)
			bool isBt = matchString(f, "0");
			skip(f, ',');	// onset time (in beats, fractional part)
			skip(f, ',');	// duration (in beats, fraction)
			f >> beat;
			if ((beat == rint(beat)) != isBt)
				throw Exception("Inconsistent beat data in match file");
			skip(f, ',');	// onset time (in beats, decimal) 
			skip(f, ',');	// offset time (in beats, decimal)
			skip(f, '[');	// additional info (e.g. melody/arpeggio/grace)
			f.getline(buf, SZ, ']');
			endPtr = buf - 1;
			do {
				element = endPtr + 1;
				if ((endPtr = strchr(element,',')) != NULL)
					*endPtr = 0;
				if (strcmp(element, "adlib") == 0)
					eventFlags |= ADLIB_FLAG;
				else if (strcmp(element, "arp") == 0)
					eventFlags |= ARPEGGIO_FLAG;
				else if (strcmp(element, "double") == 0)
					eventFlags |= DOUBLE_FLAG;	// grace notes: up main up
				else if (strcmp(element, "fermata") == 0)
					eventFlags |= FERMATA_FLAG;
				else if (strcmp(element, "grace") == 0)
					eventFlags |= GRACE_NOTE_FLAG;
				else if (strcmp(element, "le") == 0)
					eventFlags |= LEGATO_END_FLAG;
				else if (strcmp(element, "ls") == 0)
					eventFlags |= LEGATO_START_FLAG;
				else if (strcmp(element, "m") == 0)
					eventFlags |= MIDDLE_FLAG;
				else if (strcmp(element, "mord") == 0)
					eventFlags |= MORDENT_FLAG; // main up main down; or v/v
				else if (strcmp(element, "s") == 0)
					eventFlags |= MELODY_FLAG;
				else if (strcmp(element, "stacc") == 0)
					eventFlags |= STACCATO_FLAG;
				else if (strcmp(element, "trill") == 0)
					eventFlags |= TRILL_FLAG; // upper or main note start
				else if (*element != 0)
					cerr << "Warning: Bad attribute in score: " <<element<<endl;
			} while (endPtr != NULL);
			skip(f, '-');
		} else if (matchString(f, "trill(")) {
			eventFlags |= TRILL_FLAG;
			beat = UNKNOWN;
			skip(f, '-');
		} else if (matchString(f, "ornament(")) {
			eventFlags |= ORNAMENT_FLAG;
			beat = UNKNOWN;
			skip(f, '-');
		} else if (matchString(f, "trailing_played_note-") ||
				   matchString(f, "hammer_bounce-") ||   
				   matchString(f, "no_score_note-") ||
				   matchString(f, "insertion-")) {
			eventFlags |= UNSCORED_FLAG;
			beat = UNKNOWN;
		} else if (c != '%') {		// Prolog comment
			throw Exception("Parse error 4 in match file");
		}
		// READ 2nd term of Prolog expression
		if (matchString(f, "note(")) {
			skip(f, '[');	// skip identifier
			pitch = getNoteNumber(f.get());
			skip(f, ',');
			while (f.get(c) && (c != ']')) {
				switch (c) {
					case '#': pitch++; break;
					case 'b': pitch--; break;
					case 'n': break;
					default: throw Exception("Parse error 5 in match file");
				}
			}
			skip(f, ',');
			f >> octave;
			pitch += 12 * octave;
			skip(f, ',');
			f >> onset;
			skip(f, ',');
			f >> offset;
			if (versionNumber > 1.0) {
				skip(f, ',');
				f >> eOffset;
			} else
				eOffset = offset;
			skip(f, ',');
			f >> velocity;
			onset /= double(clockUnits * 1000000) / double(clockRate);
			offset /= double(clockUnits * 1000000) / double(clockRate);
			eOffset /= double(clockUnits * 1000000) / double(clockRate);
			if (timingCorrection) {
				if (displacement < 0)
					displacement = onset;
				onset -= displacement;
				offset -= displacement;
				eOffset -= displacement;
			}
			event* e = new event(onset, offset, pitch, velocity, beat);
			e->offset = eOffset;
			if (parameters->debug("eventMidi", "matlabOut") &&
					((beatLevel == 0.0) || e->isBeat(beatLevel))) {
				binaryString(buf, eventFlags, 11);
				cout << setw(10) << setprecision(6)
					 << ((eventFlags & UNSCORED_FLAG) ? -1.0 : 
						1.0 + beat / (double) numerator)
					 << "\t" << setw(7) << setprecision(3) << onset
					 << "\t" << setw(3) << velocity
					 << "\t" << setw(3) << pitch
					 << "\t" << buf << " " << noteId << "\n";
			}
			e->flags = eventFlags;
			if ((e->isMelody()&&!noMelody) || (!e->isMelody()&&!onlyMelody))
				insert(e);
			else
				delete e;
		} else if (!matchString(f, "no_played_note.") &&
				   !matchString(f, "trailing_score_note.") &&
				   !matchString(f, "deletion.") &&
				   (c != '%'))
			throw Exception("Parse error 6 in match file");
		f.ignore(SZ, '\n');
		c = f.peek();	// peek at next line to test for eof
	}
	if (!f.eof())	// check for valid reason for exit
		throw Exception("Parse error 7 in match file");
	if (parameters->debug("eventMidi", "matlabOut"))
		exit(0);
} // readMatchFile()

// Creates a percussion event for marking beat positions in MIDI files
#define DRUM_CHANNEL 9 /* NB this is channel 10 */
event* percussionEvent(double onset, double beatNumber = UNKNOWN) {
	int note = parameters->getInt("midiMetronome", 56);			// cowbell
	int vel = (note==32? 120:80);		// loud enough ??
	int trk = -1;		// corrected later
	int chan = DRUM_CHANNEL;
	int voice = 0;		// any drum kit
	event* e = new event(NOTE_ON, onset, note, vel, voice, trk, chan);
	e->beat = beatNumber;
	return e;
} // percussionEvent()

// Creates a list of MIDI percussion events for demos & aural testing
// For automatic evaluation from manually tagged (acoustic) data
void eventList :: readBeatFile(const char* fileName) {
	ifstream f(fileName);
	if (!assertWarning(f.is_open(), "Could not open beat file"))
		return;
	double beat, location;
	while (f >> beat >> location)
		add(percussionEvent(location, beat));
	replace(makeRealBeatList(1.0, true)); // interpolate missing beat posns
} // readBeatFile()

// Returns the number of MIDI tracks used (max track number, starting from 1).
//  Also places any events without a track number into the next empty track
//  or track 16 if all tracks are used.
int eventList :: countTracks() {
	int tracks = 0;
	bool flag = false;
	for (const eventList* e = next; e->ev != NULL; e = e->next) {
		if (e->ev->track > tracks)
			tracks = e->ev->track;
		else if (e->ev->track < 0)
			flag = true;
	}
	if (flag) {
		if (tracks < 16)
			tracks++;
		for (eventList* e = next; e->ev != NULL; e = e->next)
			if (e->ev->track < 0)
				e->ev->track = tracks;
	}
	return tracks;
} // countTracks()

// Outputs the list as a tmf file (for conversion to Midi using t2mf / tmf2mid).
void eventList :: writeTMF(const char* fileName, double start, double len) {
	oTMF tmf(fileName, this, start, len);
} // writeTMF()

// Creates and adds percussive events at positions specified by events in e
void eventList :: addMidiClickTrack(eventList* e) {
	deleteMidiClickTrack();
	if (e != NULL)
		for (e = e->next; e->ev != NULL; e = e->next)
			insert(percussionEvent(e->ev->onset));
} // addMidiClickTrack(eventList*)

// Creates and adds percussive events at times specified by clickLocn[]
void eventList :: addMidiClickTrack(double* clickLocn, int size) {
	deleteMidiClickTrack();
	for (int i=0; i < size; i++)
		insert(percussionEvent(clickLocn[i]));
} // addMidiClickTrack(double*,int)

// Creates and adds percussive events at times specified by clickLocn[].
//  Times are expressed as integer multiples of the parameter mult
void eventList :: addMidiClickTrack(int* clickLocations, int size, double mult){
	deleteMidiClickTrack();
	for (int i=0; i < size; i++)
		insert(percussionEvent(mult * (double)clickLocations[i]));
} // addMidiClickTrack(int*,int,double)

// Deletes all drum events from a MIDI file (drums =def channel 10)
void eventList :: deleteMidiClickTrack() {
	for (eventList* e = this; e->next->ev != NULL; )
		if (e->next->ev->channel == DRUM_CHANNEL)
			e->remove(e->next->ev);
		else
			e = e->next;
} // deleteMidiClickTrack()

// Deletes all non-drum events from a MIDI file (drums =def channel 10)
void eventList :: deleteMidiNonClickTrack() {
	for (eventList* e = this; e->next->ev != NULL; )
		if (e->next->ev->channel != DRUM_CHANNEL)
			e->remove(e->next->ev);
		else
			e = e->next;
} // deleteMidiNonClickTrack()

// For aligning with audio files. Shifts all notes so that the first note is
//  at the time given by the parameter.
void eventList :: align(double firstNote) {
	if ((nextNote() != NULL) && (firstNote >= 0)) {
		firstNote -= nextNote()->ev->onset;
		for (eventList* e = next; e->hasMore(); e = e->next) {
			e->ev->onset += firstNote;
			e->ev->offset += firstNote;
			e->ev->noteOff += firstNote;
		}
	}
} // align()

// Finds the next event in list e which is at the given rhythmic level
eventList* eventList :: nextBeat(double level) {
	eventList* e = this;
	do {
		e = e->next;
	} while ((e->ev != NULL) && ! e->ev->isBeat(level));
	return e;
} // nextBeat()

// Returns the time of the last event on the current beat
double eventList :: maxBeat() {
	double max = ev->onset;
	double beat = ev->beat;	// not all notes on same beat are contiguous
	eventList* e = next;
	while ((e->ev != NULL) && (e->ev->beat < beat + 2.0)) {
		if ((e->ev->beat==beat) && (max < e->ev->onset))
			max = e->ev->onset;
		e = e->next;
	}
	return max;
} // maxBeat()

// Returns the time of the highest note on the current beat
double eventList :: highestBeat() {
	double max = ev->onset;
	double beat = ev->beat;	// not all notes on same beat are contiguous
	int pitch = ev->pitch;
	eventList* e = next;
	while ((e->ev != NULL) && (e->ev->beat < beat + 2.0)) {
		if ((e->ev->beat==beat) && (pitch < e->ev->pitch)) {
			pitch = e->ev->pitch;
			max = e->ev->onset;
		}
		e = e->next;
	}
	return max;
} // highestBeat()

// Returns an eventList containing the beat times at a given rhythmic level;
//  interpolates missing values; uses the highest pitch note on the beat
eventList* eventList :: makeRealBeatList(double level, bool fill) {
	double min, max, high, prevMin, prevMax, prevHi, prevBeat = UNKNOWN;
	eventList* beats = new eventList();
	double delta = parameters->getDouble("delta", 0.070); // beat location resn
	if (level == UNKNOWN)
		level = 1.0;
	eventList* events = nextBeat(level);
	while (events->ev != NULL) {
		min = events->ev->onset - delta;
		max = events->maxBeat() + delta;
		high = events->highestBeat();
		int count = (int) rint((events->ev->beat - prevBeat) / level);
		if (fill && (prevBeat > UNKNOWN) && (count > 1)){ // fill in empty beats
			for (int i = 1; i < count; i++) {
				event* e = percussionEvent(high, events->ev->beat -
												 double(count - i) * level);
				e->onset = prevHi + (high-prevHi) * (double)i /(double)count;
				e->noteOff = prevMin + (min-prevMin) * (double)i /(double)count;
				e->offset = prevMax + (max-prevMax) * (double)i /(double)count;
				beats->add(e);
			}
		}
		event* e = percussionEvent(high, events->ev->beat);
		e->noteOff = min;
		e->offset = max;
		beats->add(e);
		prevBeat = events->ev->beat;
		prevMin = min;
		prevMax = max;
		prevHi = high;
		while ((events->ev != NULL) && (events->ev->beat <= prevBeat)) 
			events = events->nextBeat(level);
	}
	return beats;
} // makeRealBeatList()

// for calculating length of file, find first event with known score location
event* eventList :: firstKnown() {
	for (eventList* e = next; e->ev != NULL; e = e->next)
		if (e->ev->beat != UNKNOWN)
			return e->ev;
	return NULL;	// shouldn't reach here
} // firstKnown()

// for calculating length of file, find last event with known score location
event* eventList :: lastKnown() {
	for (eventList* e = prev; e->ev != NULL; e = e->prev)
		if (e->ev->beat != UNKNOWN)
			return e->ev;
	return NULL;	// shouldn't reach here
} // lastKnown()

// Compares two MIDI files (for testing audio transcription)
double eventList :: compare(eventList* real) {
	int fp = 0;	// count of false positives
	int fn = 0;	// count of false negatives
	int ok = 0;	// count of correctly identified notes
	double errWindow1 = parameters->getDouble("errorWindow1",0.07);
	double errWindow2 = parameters->getDouble("errorWindow2",0.25);
	eventList* detected = next;
	// uses flag for marking matched events
	for ( ; detected->ev != NULL; detected = detected->next)
		detected->ev->flags = 0;
	detected = next;
	for (real = real->next; real->ev != NULL; real = real->next) {
		detected = detected->nextEvent(real->ev->onset - errWindow2);
		while ((detected->ev != NULL) &&
				(detected->ev->onset < real->ev->onset + errWindow2)) {
			if (real->ev->match(detected->ev,errWindow1) >= TIME_ERROR) {
				ok++;
				if (parameters->debug("eventMidi", "showMatches")) {
					cout << "Match number " << ok << ":\n";
					detected->ev->print();
					real->ev->print();
				}
				break;
			} else
				detected = detected->next;
		}
	}
	for (detected = next; detected->ev != NULL; detected = detected->next)
		if (!detected->ev->isMatched())
			fp++;
	for (detected = real->next; detected->ev != NULL; detected = detected->next)
		if (!detected->ev->isMatched())
			fn++;
	if (parameters->debug("eventMidi", "showDiff")) {
		double sum = 0;
		detected = next;
		real = real->next;
		while ((real->ev != NULL) && (detected->ev != NULL)) {
			if (detected->ev->flags & PRINT_FLAG) {
				detected = detected->next;
				continue;
			} else if ((!detected->ev->isMatched()) &&
					(detected->ev->onset < real->ev->onset)) {
				cout << setw(40) << "";
				detected->ev->print(true);
				detected = detected->next;
			} else if (!real->ev->isMatched()) {
				real->ev->print(true);
				real = real->next;
			} else {
				eventList* tmp = detected;
				for ( ; tmp->ev != NULL; tmp = tmp->next)
					if ((tmp->ev->pitch == real->ev->pitch) &&
								((tmp->ev->flags & PRINT_FLAG) == 0))
						break;
				if (!assertWarning(tmp->ev!=NULL,"Event not found in compare()")
					|| !assertWarning((tmp->ev->flags & PRINT_FLAG) == 0,
								"Error reprinting event in compare()"))
					return 0;
				sum += real->ev->onset - tmp->ev->onset;
				real->ev->print(true);
				real = real->next;
				tmp->ev->print(true);
				tmp->ev->flags |= PRINT_FLAG;
			}
			cout << "\n";
		}
		while (real->ev != NULL) {
			real->ev->print(true);
			cout << "\n";
			real = real->next;
		}
		while (detected->ev != NULL) {
			if (! (detected->ev->flags & PRINT_FLAG)) {
				cout << setw(40) << "";
				detected->ev->print(true);
				cout << "\n";
			}
			detected = detected->next;
		}
		cout << setprecision(3) << "Average displacement: " << sum / double(ok)
			 << "\n";
	}
	cout << "Compare(): fp = " << setw(3) << fp
		 << ";   fn = " << setw(3) << fn
		 << ";   ok = " << setw(3) << ok
		 << ";   score = " << setw(5) << setprecision(2)
		 << double(ok) / double(ok+fp+fn) * 100.0 << "%\n";
	return double(ok) / double(ok+fp+fn);
} // compare()
