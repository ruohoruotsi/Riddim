//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: realTimeSystem.cpp
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


systemRT :: systemRT(char* optarg) {
	tiAgent = new tempoInductionAgent();
	btAgent = new beatTrackingAgent(this);
	current = NULL;
	output = new eventList();
	agentList* pastBest = new agentList();
	agent* currentBest = NULL;
	const double memory = parameters->getDouble("beatMemory", 20.0);
	if (optarg == NULL)
		return;
	parameters->add("salienceType=bounded");
	data = new eventList(optarg);
	eventList* e = data->calculateMidiSalience();
	double lastPrint = 0.0;
	for (e = e->next; e->ev != NULL; e = e->next) {
		if (e->ev->onset - lastPrint > 10.0) {
			lastPrint += 10.0;
			cout << "Progress dump to t = " << lastPrint << endl;
			tiAgent->print();
			btAgent->print();
		}
		cout << "Processing event: " << endl;
		e->ev->print();
		tiAgent->add(e->ev);
		btAgent->add(e->ev);
		btAgent->update(tiAgent, e->ev);
		btAgent->collaborate();
		// while ((pastBest->count > 0) &&
		// 	   (e->ev->onset - pastBest->ag->events->prev->ev->onset > memory))
		// 	pastBest->remove(pastBest);
		currentBest = btAgent->bestAgent();
		if (currentBest != NULL) {
		// 	pastBest->add(currentBest, false);
			if ((current == NULL) || ((currentBest != current) &&
										(currentBest->topScoreTime > 2.0))) {
				setCurrent(currentBest);
				cout << "replacing with agent " << currentBest->idNumber <<endl;
			}
			current->print(2);
		}
	}
			
	/*
		if (current == NULL)
			current = currentBest;
		else {
			int currentPosn = -1;
			int first = 0;
			int count[maxPastBest];
			int posn[maxPastBest];
			for (agentList* a = pastBest; a->ag != NULL; a = a->next, first++) {
				count[first] = 0;
				if ((currentPosn < 0) && (a->ag == current))
					currentPosn = first;
				int last = first;
				for (agentList* b = a; b != NULL; b = b->next, last++) {
					if (b == a) {
						count[first]++;
						posn[first] = last;
					}
				}
				int best = 0;
				for (int i = 0; i < first; i++) {
					if (count[i] > count[best])
						best = i;
					else if ((count[i] == count[best]) && (posn[i] > posn[best]))
						best = i;
				}
				if (count[best] > count[currentPosn]) {
					// merge using a continuity condition
					// consider metrical levels
				}
			}
		}
	}

	*/
	/*
				
		if (newBest != current) {
			cout << "change of agent" << endl;
			current = newBest;
		}
		if (current) current->print();
		tiAgent->print();
		int n = 1;
		while ((current != NULL) && (e->next->ev != NULL) &&
			   (current->beatTime + n*current->beatInterval < e->next->ev->onset)){
			output->add(new event(current->beatTime + n * current->beatInterval));
			n++;
		}
		// print();
	}
	output->print();
	delete e;
	// data->addMidiClickTrack(output);
	// data->play();

	*/

} // systemRT constructor

systemRT :: ~systemRT() {
	delete tiAgent;
	delete btAgent;
	delete data;
	delete output;
} // systemRT destructor

bool systemRT :: isCurrent(agent* ag) {
	return (current == ag);
} // isCurrent()

void systemRT :: setCurrent(agent* ag) {
	if (current != NULL)
		current->refs--;
	current = ag;
	if (current != NULL)
		current->refs++;
} // setCurrent()

void systemRT :: print() {
	cout << "Real Time System State" << endl;
	tiAgent->print();
	btAgent->print();
	output->print();
} // print()
