//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: realTime.h
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

class agent;
class event;
class agentList;
class eventList;
class systemRT;

class rtCluster {
	public:
		double count;
		double offset;
		double weightedCount;
		double weightedOffset;
		int value;

		rtCluster(int i);	
}; // class rtCluster

class tempoInductionAgent {
	private:
		double memory;
		double absoluteWindow;
		double relativeWindow;
		int minIOI;
		int maxIOI;
		int length;
		rtCluster** clusters;
		eventList* events;
		int maxClusters;
		int* rankedIndex;
		int clusterCount;

		void addPrev();
		void removeNext();
		void updateRanked();
		int windowSize(int ioi);
		void indexQuickSort(int* rankedSection, int size);

	public:
		tempoInductionAgent();
		~tempoInductionAgent();
		void add(event* e);
		void bestN(double* hypotheses, int count);
		double getWeight(double hypothesis);
		void print();
}; // class tempoInductionAgent

class beatTrackingAgent {
	private:
		systemRT* parent;
		agentList* agents;
		double currentTime;
		double deltaTime;
		double minIBI, maxIBI;
		double decayFactor;
		int hypCount;
		double* hypotheses;
		agentList* onBeat;
		int agentCount;

	public:
		beatTrackingAgent(systemRT* parent);
		~beatTrackingAgent();
		void add(event* e);
		void update(tempoInductionAgent* tiAgent, event* currentEvent);
		void collaborate();
		agent* bestAgent();
		void print();
};

class systemRT {
	private:
		tempoInductionAgent* tiAgent;
		beatTrackingAgent* btAgent;
		agent* current;
		eventList* data;
		eventList* output;

	public:
		systemRT(char* optarg = NULL);
		~systemRT();
		bool isCurrent(agent* ag);
		void setCurrent(agent* ag);
		void print();
};
