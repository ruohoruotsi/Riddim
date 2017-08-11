//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: event.h
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

#include "tmf.h"

// for beat number
#define UNKNOWN -999.123

// for event matching (must be larger than the number of harmonics)
#define TIME_ERROR 65535

// for encoding attributes of events as specified in match files
#define ADLIB_FLAG             0x1
#define ARPEGGIO_FLAG          0x2
#define DOUBLE_FLAG            0x4
#define FERMATA_FLAG           0x8
#define GRACE_NOTE_FLAG       0x10
#define MELODY_FLAG           0x20
#define MIDDLE_FLAG           0x40
#define MORDENT_FLAG          0x80
#define ORNAMENT_FLAG        0x100
#define STACCATO_FLAG        0x200
#define TRILL_FLAG           0x400
#define UNSCORED_FLAG        0x800
#define LEGATO_START_FLAG   0x1000
#define LEGATO_END_FLAG     0x2000
#define UNUSED_FLAG     0x10000000
#define UNUSED1_FLAG    0x20000000
#define PRINT_FLAG      0x40000000
#define MATCHED_FLAG    0x80000000

// for states of frequency tracks in TPV
#define PROPOSED 1
#define ALIVE 2
#define DEAD 3

// for interpolation of events at a given rhythmic level
#define INTERPOLATE_NONE 1
#define INTERPOLATE_ONCE 2
#define INTERPOLATE_ALL 4
#define BEATS_ONLY 8

// default parameter settings for beat induction
	// prev maximum speed:  200BPM = 300ms/beat
	// prev minimum speed:  100BPM = 600ms/beat
#define MIN_IBI 0.25 /* 240BPM */
#define MAX_IBI 0.75 /*  80BPM */
#define CLUSTER_WIDTH 0.025
#define MIN_CLUSTER_SIZE 0.070
#define MAX_CLUSTER_SIZE 2.500
#define TOP_N 10


// class definitions

class agentList;	// forward definition for beatInduction()
class Exception;

class event {
	public:
		double onset;		// Note onset time (seconds)
		double noteOff;		// Note offset time (seconds)
		double offset;		// Effective offset time (from pedal info) (seconds)
		int track;			// MIDI file track number
		int channel;		// MIDI channel number (0-15; note mf2t adds 1)
		int pitch;			// MIDI note or controller number
		int volume;			// MIDI velocity or parameter value
		int voice;			// Inferred from parameter changes
		int eventType;		// Usually MidiMessage (NOTE_ON, etc)
		int flags;			// MIDI or score attributes from match files
		double beat;		// From score data (in match files)
		double salience;	// Calculated from vel, pitch, vol & combinations
		char* data;			// For storing arbitrary data ** as text **
		int references;		// For memory management

		event(double on, double off=-1, int p=-1, int vol=-1, double b=UNKNOWN);
		event(int typ, double on, int p, int vel, int vox, int trk, int ch);
		event(const event& copy);
		~event();
		void init(int eType, double on, double off, double eOff, int p, int vol,
					int vox, int trk, int ch, int flgs, double bt);
		bool isBeat(double level = 1.0) const;
		int match(event* e, double errorWindow);
		void print(bool shortFormat = false) const;

		bool isAdLib() const { return (flags & ADLIB_FLAG) != 0; };
		void setAdLib() { flags |= ADLIB_FLAG; };
		bool isArpeggio() const { return (flags & ARPEGGIO_FLAG) != 0; };
		void setArpeggio() { flags |= ARPEGGIO_FLAG; };
		bool isDouble() const { return (flags & DOUBLE_FLAG) != 0; };
		void setDouble() { flags |= DOUBLE_FLAG; };
		bool isFermata() const { return (flags & FERMATA_FLAG) != 0; };
		void setFermata() { flags |= FERMATA_FLAG; };
		bool isGraceNote() const { return (flags & GRACE_NOTE_FLAG) != 0; };
		void setGraceNote() { flags |= GRACE_NOTE_FLAG; };
		bool isLegatoStart() const { return (flags & LEGATO_START_FLAG) != 0; };
		void setLegatoStart() { flags |= LEGATO_START_FLAG; };
		bool isLegatoEnd() const { return (flags & LEGATO_END_FLAG) != 0; };
		void setLegatoEnd() { flags |= LEGATO_END_FLAG; };
		bool isMelody() const { return (flags & MELODY_FLAG) != 0; };
		void setMelody() { flags |= MELODY_FLAG; };
		bool isMiddle() const { return (flags & MIDDLE_FLAG) != 0; };
		void setMiddle() { flags |= MIDDLE_FLAG; };
		bool isMordent() const { return (flags & MORDENT_FLAG) != 0; };
		void setMordent() { flags |= MORDENT_FLAG; };
		bool isOrnament() const { return (flags & ORNAMENT_FLAG) != 0; };
		void setOrnament() { flags |= ORNAMENT_FLAG; };
		bool isStaccato() const { return (flags & STACCATO_FLAG) != 0; };
		void setStaccato() { flags |= STACCATO_FLAG; };
		bool isTrill() const { return (flags & TRILL_FLAG) != 0; };
		void setTrill() { flags |= TRILL_FLAG; };
		bool isMatched() const { return (flags & MATCHED_FLAG) != 0; };
		void setMatched() { flags |= MATCHED_FLAG; };
		bool isMidiMessage() const { return eventType == CONTROLLER_CHANGE; };
		void setMidiMessage() { eventType = CONTROLLER_CHANGE; };
}; // class event

class eventList {	// doubly linked ring, with null-event terminator
	public:
		eventList* next;
		eventList* prev;
		event* ev;

	private:
		eventList(event* newEvent, eventList* p, eventList *n);
		void init(event* newEvent, eventList* p, eventList *n);
		void remove();					// disconnects from list and destroys
		void readTMF(const char* fileName);
		void readMatchFile(const char* fileName) throw (Exception);
		void readBeatFile(const char* fileName);
		void combinePedalTrack(const char* matchFileName);

	public:
		eventList();
		eventList(const char* fileName, double start=0,
					double length=-1);	// reads from tmf/beat/match files
		eventList(eventList* clone);
		~eventList();

		void add(event* newEvent);		// add to the end of the list
		void add(eventList* newEvents, double start = -1.0, double end = -1.0);
			// add to the end of the list if in given range (or range not given)
		void insert(event* newEvent);	// add, sorted by onset time
		void remove(event* oldEvent);	// removes and destroys oldEvent
		void clear(double start=0, double len=-1, int eType=0);
			// delete part or all of the list, matching eventType (0=all)
		void replace(eventList* e);		// replaces contents, destroys old
		bool hasMore() const { return ev != NULL; }
		bool isTempoChange() const { return ev &&
				(ev->eventType == META_EVENT) && (ev->pitch == SET_TEMPO); }
		bool isController() const {return ev &&
				(ev->eventType == CONTROLLER_CHANGE);}
		bool isNote() const {return ev && (ev->eventType == NOTE_ON);}
		int count(int elementType = -1) const; // returns no. of matching events
		int countTracks();	// numbers unnumbered tracks & returns total #tracks
		eventList* head();				// returns the head of the list
		eventList* nextEvent(double time);	// finds next event after time
		eventList* nearest(double time);	// finds event nearest time
		eventList* nextBeat(double level);	// finds next event on beat
		eventList* nextNote(int elementType = NOTE_ON);	// next matching event
		void align(double firstNote);	// shifts all times so first note aligns
		double maxBeat();		// finds time of last event on current beat
		double highestBeat();	// finds time of highest pitch event on curr bt
		void printStatistics(int interpolate = INTERPOLATE_NONE,
							 double start = 0, double stop = -1) const;
		void getStatistics(int interpolate, double start, double stop,
				double& oMedian, double& oMode, double& oMean, double& oSd,
				double& oAd, int& oN, int& oN1) const;
		void matlabPrint(const char *name, int interpolate = INTERPOLATE_NONE,
						 double start = 0, double stop = -1) const;
		void print() const;				// prints contents
		void reversePrint() const;		// prints contents in reverse order
		void smooth(int halfWidth);
		eventList* beatTrack();			// returns a list of tracked beat posns
		eventList* beatTrack(eventList* beats, double restart = -1.0);
		agentList* beatInduction();		// performs beat induction (clustering)
		void newInduction();			// performs beat induction (clustering)
		void learn(double level=1.0);	// learns from match files
		eventList* beatTracking(agentList* trackers, double restart = -1.0);
										// performs beat tracking

		// MIDI-related methods
		void combinePedalEvents(eventList* pedals);
		eventList* calculateMidiSalience();
		void normaliseSalience();
		void addMidiClickTrack(eventList* clickLocations);
		void addMidiClickTrack(double* clickLocations, int size);
		void addMidiClickTrack(int* clickLocations, int size, double mult);
		void deleteMidiClickTrack();	// removes channel 10 (default percn)
		void deleteMidiNonClickTrack();	// removes all channels except 10
		double getRhythmicLevel(eventList* beats);
		double getBasicTempo(bool isMode);
		eventList* makeRealBeatList(double level, bool fill);
		void evaluate(eventList* beats);
		double evaluate(const char* beatFile);
		eventList* filterEvents();
		void writeTMF(const char* fileName, double start=0, double len=-1.0);
		void oldWriteTMF(const char* fileName, double start=0, double len=-1.0);
		void play(double start=0, double len=-1.0);	// play (using Timidity)
		event* firstKnown();			// first known score event
		event* lastKnown();				// last known score event
		double compare(eventList* realEvents);
}; // class eventList

class ioiList {
	public:
		event* start;
		event* end;
		double interval;
		int cluster;
		ioiList* next;

		ioiList(event* s, event* e);
		~ioiList();
		void print() const;
}; // class ioiList

class cluster {

	public:
		ioiList* intervals;
		int size;
		double sum;
		int score;
		
		cluster();
		cluster(ioiList* ioi);
		~cluster();

		void add(ioiList* ioi);
		void add(cluster* c);
		double value() const;
		void print() const;

	private:
		void init();
}; // class cluster
