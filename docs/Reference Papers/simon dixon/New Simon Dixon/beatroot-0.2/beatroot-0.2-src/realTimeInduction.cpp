//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: realTimeInduction.cpp
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
#include "realTime.h"
#include "util.h"


rtCluster :: rtCluster(int i) {
	count = 0;
	offset = 0;
	weightedCount = 0;
	weightedOffset = 0;
	value = i;
} // rtCluster constructor

tempoInductionAgent :: tempoInductionAgent() {
	memory = parameters->getDouble("memory", 20.0);
	absoluteWindow = parameters->getDouble("absoluteWindow", 0.020);
	relativeWindow = parameters->getDouble("relativeWindow", 0.05);
	minIOI = (int) rint(parameters->getDouble("minIOI", 0.080) * 1000.0);
	maxIOI = (int) rint(parameters->getDouble("maxIOI", 2.5) * 1000.0);
	length = maxIOI - minIOI + 1;
	clusters = new (rtCluster*)[length];
	rankedIndex = new int[length];
	for (int i=0; i < length; i++) {
		clusters[i] = new rtCluster(i+minIOI);
		rankedIndex[i] = i;
	}
	events = new eventList();
	maxClusters = parameters->getInt("maxClusters",10);
	clusterCount = 0;
} // tempoInductionAgent constructor

tempoInductionAgent :: ~tempoInductionAgent() {
	for (int i=0; i < length; i++)
		delete clusters[i];
	delete[] clusters;
	delete[] rankedIndex;
	delete events;
} // tempoInductionAgent destructor

int tempoInductionAgent :: windowSize(int ioi) {
	return (int)rint(1000.0 * (absoluteWindow +
								relativeWindow * (double)ioi / 1000.0));
} // windowSize()

void tempoInductionAgent :: add(event* newEvent) {
	if (events->next->ev == NULL)		// the first event
		events->add(newEvent);
	else {
		if (!assertWarning(newEvent->onset >= events->prev->ev->onset,
				"Onset out of sequence: ignored"))
			return;
		events->add(newEvent);
		while (newEvent->onset - events->next->ev->onset > memory) {
			removeNext();
			events->remove(events->next->ev);
		}
		addPrev();
		updateRanked();
	}
} // add()

void tempoInductionAgent :: addPrev() {
	eventList* e = events->prev;
	double lastOnset = e->ev->onset;
	for (e = e->prev; e->ev != NULL; e = e->prev) {
		int ioi = (int) rint((lastOnset - e->ev->onset) * 1000.0);
		int wd = windowSize(ioi);
		int stop = ioi + wd - minIOI + 1;
		int start = ioi - wd - minIOI;
		if (ioi > maxIOI)
			break;
		if (ioi < minIOI)
			continue;
		if (start < 0)
			start = 0;
		if (stop > length)
			stop = length;
		for (int i = start; i < stop; i++) {
			clusters[i]->count += 1;
			clusters[i]->offset += ioi - minIOI - i;
		}
	}
} // addPrev()

void tempoInductionAgent :: removeNext() {
	eventList* e = events->next;
	double firstOnset = e->ev->onset;
	for (e = e->next; e->ev != NULL; e = e->next) {
		int ioi = (int) rint((e->ev->onset - firstOnset) * 1000.0);
		int wd = windowSize(ioi);
		int stop = ioi + wd - minIOI + 1;
		int start = ioi - wd - minIOI;
		if (ioi > maxIOI)
			break;
		if (ioi < minIOI)
			continue;
		if (start < 0)
			start = 0;
		if (stop > length)
			stop = length;
		for (int i = start; i < stop; i++) {
			clusters[i]->count -= 1;
			clusters[i]->offset -= ioi - minIOI - i;
		}
	}
} // removeNext()

void tempoInductionAgent :: updateRanked() {
	clusterCount = 0;
	for (int i = 0; i < length; i++) {
		clusters[i]->weightedCount = clusters[i]->count;
		clusters[i]->weightedOffset = clusters[i]->offset;
		int factor, idx;
		for (factor = 2; factor <= 8; factor++) {
			idx = (i + minIOI) * factor - minIOI;
			if (idx >= length)
				break;
			clusters[i]->weightedCount += clusters[idx]->count/(double)factor;
			clusters[i]->weightedOffset += clusters[idx]->offset/(double)factor;
		}
		for (factor = 2; factor <= 8; factor++) {
			idx = (i + minIOI) / factor - minIOI;
			if (idx < 0)
				break;
			clusters[i]->weightedCount += clusters[idx]->count/(double)factor;
			clusters[i]->weightedOffset += clusters[idx]->offset/(double)factor;
		}
//		if (clusters[i]->weightedCount > 0) {
//			int j = clusterCount;
//			for ( ; (j>0) && (clusters[i]->weightedCount >
//								clusters[rankedIndex[j-1]]->weightedCount); j--)
//				if (j < maxClusters)
//					rankedIndex[j] = rankedIndex[j-1];
//			if (j < maxClusters)
//				rankedIndex[j] = i;
//			if (clusterCount < maxClusters)
//				clusterCount++;
//		}
	}
	indexQuickSort(rankedIndex, length);
	int i;
	double sum = 0;
	for (i = 0; i < length; i++) {
		clusters[i]->value = i + minIOI;
		if (clusters[i]->weightedCount != 0)
			clusters[i]->value += (int) rint(clusters[i]->weightedOffset /
											 clusters[i]->weightedCount);
		sum += clusters[i]->weightedCount;
	}
	for (i = 0; i < length; i++)	// normalise to fractions of total
		clusters[i]->weightedCount /= sum;
	for (i = 1; i < maxClusters; i++) {
		for (int j = i; j < length; j++) {
			int k;
			for (k = 0; k < i; k++)
				if (abs(clusters[rankedIndex[k]]->value -
						clusters[rankedIndex[j]]->value)
							< windowSize(clusters[rankedIndex[j]]->value))
					break;
			if (k == i) {
				swap(rankedIndex, i, j);
				break;
			}
		}
		if (clusters[rankedIndex[i]]->weightedCount == 0)
			break;
	}
	if (clusters[rankedIndex[0]]->weightedCount == 0)
		clusterCount = 0;
	else
		clusterCount = i;
} // updateRanked()

// Sorts r[.] so that clusters[r[.]]->weightedCount is in descending order
void tempoInductionAgent :: indexQuickSort(int* r, int size) {
	int lo = 0;
	int hi = size-1;
	double pivot;
	if (hi > 0) {
		if (clusters[r[hi]]->weightedCount > clusters[r[0]]->weightedCount)
			swap(r, hi, 0);
		pivot = clusters[r[0]]->weightedCount;
		while (TRUE) {
			do {
				lo++;
			} while (clusters[r[lo]]->weightedCount > pivot);
			do {
				hi--;
			} while (clusters[r[hi]]->weightedCount < pivot);
			if (lo > hi)
				break;
			swap(r, lo, hi);
		}
		swap(r, 0, hi);
		indexQuickSort(r, hi);
		indexQuickSort(r+hi+1, size-hi-1);
	}
} // indexQuickSort()

void tempoInductionAgent :: bestN(double* hypotheses, int count) {
	for (int i = 0; i < count; i++) {
		if (i < clusterCount) {
			hypotheses[i] = 0.001 * (double)clusters[rankedIndex[i]]->value;
			// weights[i] = clusters[rankedIndex[i]]->weightedCount;
		} else {
			hypotheses[i] = 0;
			// weights[i] = 0;
		}
	}
} // bestN()

double tempoInductionAgent :: getWeight(double hypothesis) {
	int index = (int) rint(hypothesis * 1000);
	if ((index < minIOI) || (index > maxIOI))
		return 0;
	int min = index - minIOI;
	for (int i = index - minIOI - 20; i <= index - minIOI + 20; i++) {
		if (i < 0)
			continue;
		if (i >= length)
			break;
		if (abs(index - clusters[i]->value) < abs(index - clusters[min]->value))
			min = i;
	}
	return clusters[min]->weightedCount;
} // getWeight()

void tempoInductionAgent :: print() {
	cout << "Top " << clusterCount << " clusters:\n";
	for (int i = 0; i < clusterCount; i++)
		cout << setw(2) << i+1
			 << "  " << setw(5) << clusters[rankedIndex[i]]->value
			 << " ms    Score=" << setprecision(0) << setw(3)
			 << clusters[rankedIndex[i]]->weightedCount * 100000.0
			 << "  i=" << setw(5) << rankedIndex[i] + minIOI << endl;
} // print()
