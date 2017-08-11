//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: agent.cpp
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

#include "agent.h"
#include "event.h"
#include "includes.h"
#include "param.h"
#include "realTime.h"
#include "util.h"


// Implementations of the agent and agentList classes for beat tracking

#define POST_MARGIN_FACTOR 0.4
#define PRE_MARGIN_FACTOR 0.2
#define MAX_CHANGE 0.2
#define CONF_FACTOR 0.5
#define DEFAULT_CORRECTION_FACTOR 50.0	/* was 20.0 */
#define INNER_MARGIN 0.040
#define DEFAULT_EXPIRY_TIME 5.0


// CLASS AGENT: Constructors and destructor

int agent :: idCounter = 0;
double agent :: innerMargin = INNER_MARGIN;
double agent :: correctionFactor = DEFAULT_CORRECTION_FACTOR;
double agent :: expiryTime = DEFAULT_EXPIRY_TIME;
double agent :: decayFactor = 0;	// was 10 (for RT BT)

agent :: agent(double beatInt) {
	init(beatInt);
}

agent :: agent(agent* clone) {
	idNumber = idCounter++;
	phaseScore = clone->phaseScore;
	tempoScore = clone->tempoScore;
	topScoreTime = clone->topScoreTime;
	beatCount = clone->beatCount;
	beatInterval = clone->beatInterval;
	initialBeatInterval = clone->initialBeatInterval;
	beatTime = clone->beatTime;
	events = new eventList(clone->events);
	postMargin = clone->postMargin;
	preMargin = clone->preMargin;
}

void agent :: init(double beatInt) {
	static double preMarginFactor =
				parameters->getDouble("preMarginFactor", PRE_MARGIN_FACTOR);
	static double postMarginFactor =
				parameters->getDouble("postMarginFactor", POST_MARGIN_FACTOR);
	innerMargin = parameters->getDouble("innerMargin", innerMargin);
	correctionFactor=parameters->getDouble("correctionFactor",correctionFactor);
	expiryTime = parameters->getDouble("expiryTime", expiryTime);
	beatInterval = beatInt;
	initialBeatInterval = beatInt;
	postMargin = beatInt * postMarginFactor;
	preMargin = beatInt * preMarginFactor;
	idNumber = idCounter++;
	refs = 0;
	phaseScore = 0.0;
	tempoScore = 0.0;
	topScoreTime = 0.0;
	beatCount = 0;
	beatTime = -1.0;
	events = new eventList();
} // init()

agent :: ~agent() {
	delete(events);
}

// CLASS AGENT: methods

void agent :: print(int level) {
	cout << "\tAg#" << setw(4) << idNumber << ": "
		 << setprecision(3) << beatInterval;
	if (level >= 1)
		cout << "  Beat#" << setw(3) << beatCount
			 << "  Time=" << setw(7) << beatTime
			 << "  Score=T" << setprecision(2) << setw(4) << tempoScore
			 << ":P" << setw(4) << phaseScore
			 << ":" << setprecision(1) << setw(3) << topScoreTime;
	if (level >= 2)
		cout << endl;
	if (level >= 3)
		events->print();
} // print()

void agent :: accept(event* e, double err, int beats) {
	beatTime = e->onset;
	events->add(e);
	if (fabs(initialBeatInterval - beatInterval - err / correctionFactor) <
					MAX_CHANGE * initialBeatInterval)
		beatInterval += err / correctionFactor;// Adjust tempo using err
	// if (err > 0) {
	// 	phaseScore += (1.0 - CONF_FACTOR * err / postMargin) * e->salience;
	// } else {
	// 	phaseScore += (1.0 + CONF_FACTOR * err / preMargin) * e->salience;
	// }
	beatCount += beats;
	double conFactor = 1.0 - CONF_FACTOR * err/ (err>0? -postMargin: preMargin);
	if (decayFactor > 0) {
		double memFactor = 1. - 1. / threshold((double)beatCount,1,decayFactor);
		phaseScore = memFactor * phaseScore +
					 (1.0 - memFactor) * conFactor * e->salience;
	} else
		phaseScore += conFactor * e->salience;
	if (parameters->debug("agent", "showAccepted")) {
		print(1);
		cout << "  Err=" << (err<0?"":"+") << setprecision(3) << setw(5) << err
			 << (fabs(err) > innerMargin ? '*':' ') << endl;
	}
} // accept()

bool agent :: trackBeats(event* e, agentList* a) {
	double err;
	if (beatTime < 0) {	// first event
		accept(e, 0, 1);
		return true;
	} else {			// subsequent events
		if (e->onset - events->prev->ev->onset > expiryTime) {
			phaseScore = -1.0;	// flag agent to be deleted
			return false;
		}
		double beats = rint((e->onset - beatTime) / beatInterval);
		err = e->onset - beatTime - beats * beatInterval;
		if ((beats > 0) && (-preMargin <= err) && (err <= postMargin)) {
			if (fabs(err) > innerMargin)	// Create new agent that skips this
				a->add(new agent(this));	//  event (avoids large phase jump)
			accept(e, err, (int)beats);
			return true;
		}
	}
	return false;
} // agent::trackBeats()

// interpolates for missing beats
void agent :: fillBeats(double start) {
	double prevBeat, nextBeat, currentInterval, beats;
	eventList* list = events->next;
	if (list->ev != NULL)
		prevBeat = list->ev->onset;
		// prevBeat = fmod(list->ev->onset, beatInterval);	// fill from 0
	for ( ; list->ev != NULL; list = list->next) {
		nextBeat = list->ev->onset;
		beats = rint((nextBeat - prevBeat) / beatInterval - 0.01); //prefer slow
		currentInterval = (nextBeat - prevBeat) / beats;
		for ( ; (nextBeat > start) && (beats > 1.5); beats--) {
			prevBeat += currentInterval;
			if (parameters->debug("agent", "showFilledBeats"))
				cout << "Insert beat at:"
					 << setw(8) << setprecision(3) << prevBeat
					 << " (n=" << setprecision(0) << beats - 1.0 << ")" << endl;
			list->add(new event(prevBeat));
		}
		prevBeat = nextBeat;
	}
} // fillBeats()

// Show results from this agent
void agent :: showTracking(eventList* all, double level) {
	int count = 1, gapCount;
	double prevBeat, nextBeat, gap;
	eventList* beats = events->next;		// point to 1st beat
	all = all->next;						// point to 1st event
	if (!assertWarning(beats->ev != NULL, "showTracking(): No beats found"))
		return;
	prevBeat = beats->ev->onset;
	// prevBeat = fmod(beats->ev->onset, beatInterval);
	cout << setprecision(3) << "Beat  (IBI)   BeatTime   Other Events";
	bool first = true;
	for ( ; all->ev != NULL; all = all->next) {	// print each real event
		while ((beats->ev != NULL) &&		// if event was chosen as beat
				(beats->ev->onset <= all->ev->onset + CLUSTER_WIDTH)) {
			gap = beats->ev->onset - prevBeat;
			gapCount = (int) rint(gap / beatInterval);
			for (int j = 1; j < gapCount; j++) {	//empty beat(s) before event
				nextBeat = prevBeat + gap / (double)gapCount;
				cout << endl << setw(4) << count++
					 << " (" << setw(5) << (nextBeat - prevBeat)
					 << ") [" << setw(7) << nextBeat << " ]";
				prevBeat = nextBeat;
			}
			cout << endl << setw(4) << count++
				 << " (" << setw(5) << (all->ev->onset - prevBeat) << ") ";
			prevBeat = beats->ev->onset;
			beats = beats->next;
			first = false;
		}
		if ((beats->ev != NULL) && (beats->ev->onset > all->ev->onset)) {
			gap = beats->ev->onset - prevBeat;
			gapCount = (int) rint(gap / beatInterval);
			for (int j = 1; j < gapCount; j++) {	//empty beat(s) before event
				nextBeat = prevBeat + gap / (double)gapCount;
				if (nextBeat >= all->ev->onset)
					break;
				cout << endl << setw(4) << count++
					 << " (" << setw(5) << (nextBeat - prevBeat)
					 << ") [" << setw(7) << nextBeat << " ]";
				prevBeat = nextBeat;
			}
			first = false;
		}
		if (first)	// for correct formatting of any initial (pre-beat) events
			cout << "\n                       ";
		cout << setw(8) << all->ev->onset << (all->ev->isBeat(level)?"* ":"  ");
		first = false;
	}
	cout << endl;
} // showTracking()

// End of CLASS AGENT


// CLASS AGENTLIST: Constructor and Destructor

// Linked list terminated by an agentList with a NULL agent (ag)
agentList :: agentList(agent* a, agentList* al) {
	ag = a;
	next = al;
	if (next == NULL) {
		count = 0;
		thresholdBI = parameters->getDouble("thresholdBI", 0.02);
		thresholdBT = parameters->getDouble("thresholdBT", 0.04);
	}
}

agentList :: ~agentList() {
	if ((ag != NULL) && (! --ag->refs))
		delete ag;
	if (next != NULL)
		delete next;
}


// CLASS AGENTLIST: Methods

void agentList :: print() {
	cout << "agentList.print: (size=" << count << ")" << endl;
	for (agentList* ptr = this; ptr->ag != NULL; ptr = ptr->next)
		ptr->ag->print(2);
	cout << "End of agentList.print()" << endl;
} // print()

// Appends newAgent to list (sort==false), or inserts newAgent into the list
//  in ascending order of beatInterval
void agentList :: add(agent* newAgent, bool sort){
	if (newAgent == NULL)
		return;
	agentList* ptr;
	count++;
	newAgent->refs++;
	for (ptr = this; ptr->ag != NULL; ptr = ptr->next)
		if (sort && (newAgent->beatInterval <= ptr->ag->beatInterval)) {
			ptr->next = new agentList(ptr->ag, ptr->next);
			ptr->ag = newAgent;
			return;
		}
	ptr->next = new agentList();
	ptr->ag = newAgent;
} // add()

void agentList :: sort() { // bubble sort, since list is almost sorted
	bool sorted = false;
	while (!sorted) {
		sorted = true;
		for (agentList *ptr = this; ptr->ag != NULL; ptr = ptr->next) {
			if ((ptr->next->ag != NULL) &&
					(ptr->ag->beatInterval > ptr->next->ag->beatInterval)) {
				agent* temp = ptr->ag;
				ptr->ag = ptr->next->ag;
				ptr->next->ag = temp;
				sorted = false;
			}
		} // for
	} // while
} // sort()

void agentList :: remove(agentList* ptr) {
	count--;
	if (parameters->debug("agent", "showDeleted"))
		cout << "Deleting agent " << ptr->ag->idNumber << endl;
	if (! --ptr->ag->refs)
		delete(ptr->ag);
	ptr->ag = ptr->next->ag;
	agentList* tmp = ptr->next;
	ptr->next = ptr->next->next;
	tmp->next = NULL;
	tmp->ag = NULL;
	delete(tmp);
} // remove()

void agentList :: removeDuplicates(systemRT* sys) {
	sort();
	for (agentList *ptr = this; ptr->ag != NULL; ptr = ptr->next) {
		if (ptr->ag->phaseScore < 0.0)
			continue;
		for (agentList *ptr2 = ptr->next; ptr2->ag != NULL; ptr2 = ptr2->next) {
			if (ptr2->ag->beatInterval - ptr->ag->beatInterval > thresholdBI)
				break;
			if (fabs(ptr->ag->beatTime - ptr2->ag->beatTime) > thresholdBT)
				continue;
			if (ptr->ag->phaseScore < ptr2->ag->phaseScore) {
				ptr->ag->phaseScore = -1.0;		// flag for deletion
				if (ptr2->ag->topScoreTime < ptr->ag->topScoreTime)
					ptr2->ag->topScoreTime = ptr->ag->topScoreTime;
				if ((sys != NULL) && sys->isCurrent(ptr->ag)) {
					sys->setCurrent(ptr2->ag);
					cout << "mutating to agent " << ptr2->ag->idNumber << endl;
				}
				break;
			} else {
				ptr2->ag->phaseScore = -1.0;	// flag for deletion
				if (ptr->ag->topScoreTime < ptr2->ag->topScoreTime)
					ptr->ag->topScoreTime = ptr2->ag->topScoreTime;
				if ((sys != NULL) && sys->isCurrent(ptr2->ag)) {
					sys->setCurrent(ptr->ag);
					cout << "mutating to agent " << ptr->ag->idNumber << endl;
				}
			}
		}
	}
	for (agentList *ptr = this; ptr->ag != NULL; ) {
		if (ptr->ag->phaseScore < 0.0) {
			if ((sys != NULL) && sys->isCurrent(ptr->ag)) {
				sys->setCurrent(NULL);
				cout << "current is deleted ... must have expired" << endl;
			}
			remove(ptr);
		} else
			ptr = ptr->next;
	}
	if (parameters->debug("agent", "showDeleted")) {
		print();
	}
} // agentList::removeDuplicates()

void agentList :: beatTrack(eventList* el) {
	eventList* ptr;
	bool phaseGiven = (ag != NULL) && (ag->beatTime >= 0); // if given for one,
	bool created = phaseGiven;							   // assume for others
	if (parameters->debug("agent", "showInput"))
		el->print();
	double prevBeatInterval = -1.0;
	for (ptr = el->next; ptr->ev != NULL; ptr = ptr->next) {
		for (agentList* ap = this; ap->ag != NULL; ap = ap->next) {
			agent* currentAgent = ap->ag;
			if (currentAgent->beatInterval != prevBeatInterval) {
				if ((prevBeatInterval>=0) && !created && (ptr->ev->onset<5.0)) {
					// Create new agent with different phase
					agent* newAgent = new agent(prevBeatInterval);
					newAgent->trackBeats(ptr->ev, this);
					add(newAgent);
				}
				prevBeatInterval = currentAgent->beatInterval;
				created = phaseGiven;
			}
			if (currentAgent->trackBeats(ptr->ev, this))
				created = true;
			if (currentAgent != ap->ag)	// new one been inserted, skip it
				ap = ap->next;
		} // loop for each agent
		removeDuplicates();
	} // loop for each event
} // beatTrack()

agent* agentList :: bestAgent() {
	bool av = !strcmp(parameters->getString("averageSalience", "off"), "on");
	double best = -1.0;
	agent* bestAg = NULL;
	for (agentList* ap = this; ap->ag != NULL; ap = ap->next) {
		double startTime = ap->ag->events->next->ev->onset;
		double conf = (ap->ag->phaseScore + ap->ag->tempoScore) /
						 (av? (double)ap->ag->beatCount: 1.0);
		if (conf > best) {
			bestAg = ap->ag;
			best = conf;
		}
		if (parameters->debug("agent", "showResults")) {
			ap->ag->print(0);
			cout << setprecision(3) << " +" << startTime
				 << setprecision(1) << "    Av-salience = " << conf << endl;
			if (parameters->debug("agents", "chooseAgent")) {
				char c = cin.get();
				cin.ignore(1000, '\n');	// skip to newline
				cin.ignore();			// skip newline
				if (c == 'y')
					return ap->ag;
			}
		}
	}
	if (parameters->debug("agent", "showResults")) {
		if (bestAg != NULL) {
			cout << "Best ";
			bestAg->print(0);
			cout << "    Av-salience = "
				 << setw(5) << setprecision(1) << best << endl;
			bestAg->events->print();
		} else
			cout << "No surviving agent - beat tracking failed" << endl;
	}
	return bestAg;
} // agentList::bestAgent()

