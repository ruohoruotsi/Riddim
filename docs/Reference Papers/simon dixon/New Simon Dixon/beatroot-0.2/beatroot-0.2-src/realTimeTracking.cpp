//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: realTimeTracking.cpp
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


beatTrackingAgent :: beatTrackingAgent(systemRT* p) {
	parent = p;
	agents = new agentList();
	currentTime = -1;
	deltaTime = 0;
	minIBI = parameters->getDouble("minIBI", 0.250);
	maxIBI = parameters->getDouble("maxIBI", 0.750);
	decayFactor = parameters->getDouble("decayFactor", 0.9);
	hypCount = parameters->getInt("inductionHypotheses", 20);
	hypotheses = new double[hypCount];
	onBeat = new agentList();
} // beatTrackingAgent constructor

beatTrackingAgent :: ~beatTrackingAgent() {
	delete agents;
	delete[] hypotheses;
	delete onBeat;
} // beatTrackingAgent destructor

void beatTrackingAgent :: add(event* e) {
	if (currentTime >= 0)
		deltaTime = e->onset - currentTime;
	for (agentList* a = agents; a->ag != NULL; a = a->next) {
		agent* currentAgent = a->ag;
		currentAgent->tempoScore *= pow(decayFactor, deltaTime);
		if (currentAgent->trackBeats(e, agents))
			; //	onBeat->add(currentAgent);
		if (currentAgent != a->ag)
			a = a->next;
	}
	agents->removeDuplicates(parent);
	currentTime = e->onset;
	delete onBeat;
	onBeat = new agentList();
	for (agentList* a = agents; a->ag != NULL; a = a->next) {
		if (a->ag->beatTime == currentTime)
			onBeat->add(a->ag);
	}
} // add()

// Reward the tracking agents which agree with the clustering agent, and
// create new tracking agents for important clusters that have no agent
void beatTrackingAgent :: update(tempoInductionAgent* tiAgent,
								 event* currentEvent) {
	for (agentList* a = agents; a->ag != NULL; a = a->next) {
		a->ag->tempoScore += tiAgent->getWeight(a->ag->beatInterval);
	}
	tiAgent->bestN(hypotheses, hypCount);
	for (int i = 0; i < hypCount; i++) {
		if (hypotheses[i] == 0)
			break;
		if ((hypotheses[i] < minIBI) || (hypotheses[i] > maxIBI))
			continue;
		bool found = false;
		for (agentList* a = agents; a->ag != NULL; a = a->next) {
			if (fabs(a->ag->beatInterval-hypotheses[i]) < agents->thresholdBI) {
				found = true;
				break;
			}
			if (a->ag->beatInterval > hypotheses[i])
				break;
		}
		if (!found) {
			agent* newAgent = new agent(hypotheses[i]);
			agents->add(newAgent);
			newAgent->trackBeats(currentEvent, agents);
		}
	}
} // update()

// Reward agents which agree at integer multiples of tempos
void beatTrackingAgent :: collaborate() {
	const int maxCollaboration = 8;
	double mul[onBeat->count][maxCollaboration];
	double sub[onBeat->count][maxCollaboration];
	int i = 0;
	for (i = 0; i < onBeat->count; i++)
		for (int j = 0; j < maxCollaboration; j++)
			mul[i][j] = sub[i][j] = 0;
	i = 0;
	for (agentList* a = onBeat; a->ag != NULL; i++, a = a->next) {
		int j = 0;
		for (agentList* b = onBeat; b->ag != NULL; j++, b = b->next) {
			if (b->ag->events->prev->prev->ev == NULL)
				continue;
			double ratio = rint(b->ag->beatInterval / a->ag->beatInterval);
			if (ratio <= 1.0)
				continue;
			if (ratio > maxCollaboration)
				break;
			double err = b->ag->beatInterval - ratio * a->ag->beatInterval;
			if (err > b->ag->innerMargin)
				continue;
			eventList* e = a->ag->events->prev;
			for (int k=0; k < ratio; k++)
				e = e->prev;
			if ((e->ev != NULL) &&
				  (b->ag->events->prev->prev->ev->onset == e->ev->onset)) {
				if (mul[i][(int)ratio-1] < b->ag->tempoScore)
					mul[i][(int)ratio-1] = b->ag->tempoScore;
				if (sub[j][(int)ratio-1] < a->ag->tempoScore)
					sub[j][(int)ratio-1] = a->ag->tempoScore;
			}
		}
	}
	i = 0;
	double oldSum = 0;
	double newSum = 0;
	for (agentList* a = onBeat; a->ag != NULL; i++, a = a->next) {
		oldSum += a->ag->tempoScore;
		for (int j = 1; j < maxCollaboration; j++) {
			a->ag->tempoScore += mul[i][j];
			a->ag->tempoScore += sub[i][j];
		}
		newSum += a->ag->tempoScore;
	}
	if (oldSum > 0)
		newSum /= oldSum;
	if (newSum > 0)
		for (agentList* a = onBeat; a->ag != NULL; i++, a = a->next)
			a->ag->tempoScore /= newSum;
} // collaborate()

agent* beatTrackingAgent :: bestAgent() {
	agent* best = agents->bestAgent();
	double bestTopScoreTime = 0;
	agent* topScorer = best;
	bool found = false;
	for (agentList* a = agents; a->ag != NULL; a = a->next) {
		if (a->ag == best) {
			found = true;
			a->ag->topScoreTime += deltaTime;
		} else if (a->ag->topScoreTime > deltaTime)
			a->ag->topScoreTime -= deltaTime / 2.0;
		else
			a->ag->topScoreTime = 0;
		if (a->ag->topScoreTime > bestTopScoreTime) {
			topScorer = a->ag;
			bestTopScoreTime = a->ag->topScoreTime;
		}
	}
	if (best != NULL) {
		cout << "Best ";
		best->print(2);
	}
	if (!found)
		cout << "Best not found at t = " << currentTime << endl;
	return topScorer;
} // bestAgent()

void beatTrackingAgent :: print() {
    cout << "There are currently " << agents->count << " agents:" << endl;
	for (agentList* a = agents; a->ag != NULL; a = a->next)
		if (a->ag->topScoreTime > 0)
			a->ag->print(3);	// show beat list only for top agents
		else
			a->ag->print(2);
} // print()
