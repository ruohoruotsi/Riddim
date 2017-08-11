//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: agent.h
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

#ifndef NULL
#define NULL 0
#endif

class agentList;
class event;
class eventList;
class systemRT;

class agent {
	public:
		double preMargin;				// default value was 0.040 (0.020)
		double postMargin;				// default value was 0.040
		static double innerMargin;		// default value was 0.040
		static double correctionFactor;	// default value was 30.0
		static double expiryTime;		// default value was 3.0 ( * 5.0 )
		static double decayFactor;
		static int idCounter;
		int idNumber;
		int refs;
		double tempoScore;
		double phaseScore;
		double topScoreTime;	// how long has this agent been the best
		int beatCount;
		double beatInterval;
		double initialBeatInterval;
		double beatTime;
		eventList* events;

		agent(double beatInt);
		agent(agent* clone);
		~agent();

		bool trackBeats(event* e, agentList* a);
		void fillBeats(double start = -1.0);
		void showTracking(eventList* e, double level = 1.0);
		void print(int level = 100);
	private:
		void init(double beatInt = 0.0);
		void accept(event* e, double err, int beats);
};

class agentList {
	public:
		agent* ag;
		agentList* next;
		int count;
		double thresholdBI;
		double thresholdBT;

		agentList(agent* a = NULL, agentList* al = NULL);
		~agentList();
		void add(agent* a, bool sort = true);
		void sort();
		void remove(agentList* ptr);
		void removeDuplicates(systemRT* sys = NULL);
		void beatTrack(eventList* el);
		agent* bestAgent();
		void print();
};
