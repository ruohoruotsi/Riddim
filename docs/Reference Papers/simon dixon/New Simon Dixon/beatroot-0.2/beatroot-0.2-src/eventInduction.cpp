//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: eventInduction.cpp
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
#include "agent.h"
#include "event.h"
#include "param.h"
#include "util.h"

agentList* eventList :: beatInduction() {
	int i, j, b, submult,
		intervals = 0,			// number of interval clusters
		bestn[TOP_N], bestCount;// index and count of high-scoring clusters
	double ratio, err, degree;
	eventList* ptr;
	ioiList* ioi;
	double width = parameters->getDouble("clusterWidth", CLUSTER_WIDTH);
	double minCluster = parameters->getDouble("minCluster", MIN_CLUSTER_SIZE);
	double maxCluster = parameters->getDouble("maxCluster", MAX_CLUSTER_SIZE);
	double minIBI = parameters->getDouble("minIBI", MIN_IBI);
	double maxIBI = parameters->getDouble("maxIBI", MAX_IBI);
	int clusterCount = (int) ceil((maxCluster - minCluster) / width);
	cluster* clusters[clusterCount];
	
	if (parameters->debug("eventInduction", "showEvents")) {
		cout << "beatInduction(): events to be processed:\n";
		print();
	}
	for (eventList* events = next; events->ev != NULL; events = events->next) {
		for (ptr = events->next; ptr->ev != NULL; ptr = ptr->next) {
			ioi = new ioiList(events->ev, ptr->ev);
			if (ioi->interval < minCluster)	{	// skip short intervals
				delete ioi;
				continue;
			}
			if (ioi->interval > maxCluster) { // ioi too long
				delete ioi;
				break;
			}
			for (b = 0; b < intervals; b++)			// otherwise classify them
				if (fabs(clusters[b]->value() - ioi->interval) < width) {
					if ((b < intervals - 1) &&
							fabs(clusters[b+1]->value() - ioi->interval) <
							fabs(clusters[b]->value() - ioi->interval))
						b++;		// next class is closer
					clusters[b]->add(ioi);
					break;
				}
			if (b == intervals) {	// no suitable class found; insert new one
				if (!assertWarning(intervals++ < clusterCount,
						"beatInduction(): Too many clusters"))
					continue;
				for (; (b>0) && (clusters[b-1]->value() > ioi->interval); b--) {
					clusters[b] = clusters[b-1];
				}
				clusters[b] = new cluster(ioi);
			}
		}
	}
	if (parameters->debug("eventInduction", "showClusters")) {
		cout << "Inter-onset interval histogram:\n"
				"StartMatlabCode\n"
				"ioi = [\n";
		for (b = 0; b < intervals; b++)
			cout << setw(4) << b << " "
				 << setw(7) << setprecision(3) << clusters[b]->value() << " "
				 << setw(7) << clusters[b]->size << "\n";
		cout << "]; ioiclusters(ioi, name);\n"
				"EndMatlabCode\n";
	}
	for (b = 0; b < intervals; b++)	// merge similar intervals
	// needs to be changed - they are now in order, so don't need the 2nd loop
	// should instead check BOTH sides before averaging or upper gps don't work
		for (i = b+1; i < intervals; i++)
			if (fabs(clusters[b]->value() - clusters[i]->value()) < width){
				clusters[b]->add(clusters[i]);
				--intervals;
				for (j=i+1; j<=intervals; j++) {
					clusters[j-1] = clusters[j];
				}
			}
	if (intervals == 0)
		return new agentList();
	for (b = 0; b < intervals; b++)
		clusters[b]->score = 10 * clusters[b]->size;
	bestn[0] = 0;
	bestCount = 1;
	for (b = 0; b < intervals; b++)
		for (i = 0; i <= bestCount; i++)
			if ((i<TOP_N) && ((i == bestCount) ||
							 (clusters[b]->score > clusters[bestn[i]]->score))){
				if (bestCount < TOP_N)
					bestCount++;
				for (j = bestCount - 1; j > i; j--)
					bestn[j] = bestn[j-1];
				bestn[i] = b;
				break;
			}
	if (parameters->debug("eventInduction", "showBestClusters")) {
		cout << "Best " << bestCount << ": (before)\n";
		for (b = 0; b < bestCount; b++)
			cout << setprecision(3) << clusters[bestn[b]]->value() << " : "
				 << clusters[bestn[b]]->score << "\n";
	}
	for (b = 0; b < intervals; b++)	// score intervals
		for (i = b+1; i < intervals; i++) {
			ratio = clusters[b]->value() / clusters[i]->value();
			if (submult = (ratio < 1))
				degree = rint(1/ratio);
			else
				degree = rint(ratio);
			if ((degree >= 2.0) && (degree <= 8.0)) {
				if (submult)
					err = fabs(clusters[b]->value() * degree -
							   clusters[i]->value());
				else
					err = fabs(clusters[b]->value() -
							   clusters[i]->value() * degree);
				if (err < (submult? width : width * degree)) {
					if (degree >= 5.0)
						degree = 1.0;
					else
						degree = 6.0 - degree;
					clusters[b]->score += (int)rint(degree) * clusters[i]->size;
					clusters[i]->score += (int)rint(degree) * clusters[b]->size;
				}
			}
		}
	if (parameters->debug("eventInduction", "showBestClusters")) {
		cout << "Best " << bestCount << ": (after)\n";
		for (b = 0; (b < bestCount); b++)
			cout << setprecision(3) << clusters[bestn[b]]->value() << " : "
				 << clusters[bestn[b]]->score << "\n";
	}
	if (parameters->debug("eventInduction", "showClusterScores")) {
		cout << "Inter-onset interval histogram 2:\n";
		for (b = 0; b < intervals; b++)
			cout << setw(3) << b << ": "
				 << setprecision(3) << clusters[b]->value() << " : "
				 << setw(3) << clusters[b]->size
				 << " (score: " << clusters[b]->score << ")\n";
	}

	agentList* a = new agentList();
	a->count = 0;
	for (int index = 0; index < bestCount; index++) {
		b = bestn[index];
		// Adjust it, using the size of super- and sub-intervals
		double newSum = clusters[b]->value() * (double)clusters[b]->score;
		int newCount = clusters[b]->size;
		int newWeight = clusters[b]->score;
		for (i = 0; i < intervals; i++) {
			if (i == b)
				continue;
			ratio = clusters[b]->value() / clusters[i]->value();
			if (ratio < 1) {
				degree = rint(1/ratio);
				if ((degree >= 2.0) && (degree <= 8.0)) {
					err = fabs(clusters[b]->value() * degree -
							   clusters[i]->value());
					if (err < width) {
						newSum += clusters[i]->value() / degree *
								   (double)clusters[i]->score;
						newCount += clusters[i]->size;
						newWeight += clusters[i]->score;
					}
				}
			} else {
				degree = rint(ratio);
				if ((degree >= 2.0) && (degree <= 8.0)) {
					err = fabs(clusters[b]->value() -
							   degree * clusters[i]->value());
					if (err < width * degree) {
						newSum += clusters[i]->value() * degree *
								   (double)clusters[i]->score;
						newCount += clusters[i]->size;
						newWeight += clusters[i]->score;
					}
				}
			}
		}
		double beat = newSum / (double)newWeight;
		// Scale within range
		while (beat < minIBI)		// Maximum speed
			beat *= 2.0;
		while (beat > maxIBI)		// Minimum speed
			beat /= 2.0;
		if (beat >= minIBI) {
			a->add(new agent(beat));
			if (parameters->debug("eventInduction", "IBI"))
				cout << " " << setprecision(3) << beat;
		}
	}
	if (parameters->debug("eventInduction", "IBI"))
		cout << " IBI\n";
	for (int i = 0; i < intervals; i++)
		delete clusters[i];
	return a;
} // beatInduction()

int top(int low) {
	return low + 25; // low/10;
}

void eventList :: newInduction() {
	const int MAX_MS = 2500;
	int count[MAX_MS];
	for (int i=0; i < MAX_MS; i++)
		count[i] = 0;
	for (eventList* ptr1 = next; ptr1->ev != NULL; ptr1 = ptr1->next)
		for (eventList* ptr2 = ptr1->next; ptr2->ev != NULL; ptr2 = ptr2->next){
			int diff = int(rint((ptr2->ev->onset - ptr1->ev->onset)*1000.0));
			if (diff < MAX_MS)
				count[diff]++;
			else break;
		}
	int clnum;
	const int MAX_CL = 10;
	int cluster[MAX_CL];
	int csize[MAX_CL];
	for (clnum = 0; clnum < MAX_CL; clnum++) {
		int sum = 0;
		int max = 0;
		int maxp = 0;
		int hi = 70;
		int lo = hi;
		while (hi < MAX_MS) {
			if (hi >= top(lo))
				sum -= count[lo++];
			else {
				sum += count[hi++];
				if (sum > max) {
					max = sum;
					maxp = lo;
				}
			}
		}
		if (max == 0)
			break;
		hi = top(maxp);
		if (hi > MAX_MS)
			hi = MAX_MS;
		int cnt = sum = 0;
		for (lo = maxp; lo < hi; lo++) {
			sum += lo * count[lo];
			cnt += count[lo];
			count[lo] = 0;
		}
		assertWarning(cnt == max, "Rounding error in newInduction");
		cluster[clnum] = sum / cnt;
		csize[clnum] = cnt;
		cout << " " << setprecision(3) << double(sum/cnt)/1000.0;
		//cout << "Cluster " << clnum+1 << ": " << sum/cnt << "ms ("
		//	 << cnt << " intervals)\n";
	}
	cout << " IBI\n";
	// cout << "END OF NEW_INDUCTION\n";
} // newInduction()
