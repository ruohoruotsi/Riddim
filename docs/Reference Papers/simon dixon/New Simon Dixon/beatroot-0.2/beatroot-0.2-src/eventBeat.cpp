//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: eventBeat.cpp
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


inline int wrapIndex(int in, int sz) {
	return (in < 0)? -1 - in: ((in >= sz)? 2 * sz - in - 1: in);
	// return (in < 0)? 0: ((in >= sz)? sz - 1: in);
}

void eventList :: smooth(int halfWidth) {
	int direction = (halfWidth < 0)? -1: 1;
	halfWidth *= direction;
	int sz = count(NOTE_ON) - 1;
	if (!assertWarning(sz > halfWidth, "Too few beats to smooth"))
		return;
	eventList* e = nextNote();
	double ibi[sz];
	for (int i = 0; i < sz; i++, e = e->nextNote())
		ibi[i] = e->nextNote()->ev->onset - e->ev->onset;
	double mem = 0;
	for (int i = -halfWidth; i <= halfWidth; i++)
		mem += ibi[wrapIndex(i,sz)];
	double div = 2 * halfWidth + 1;
	double smoothed[sz];
	for (int i = 0; i < sz; i++) {
		double diff = ibi[i] - mem / div;
		smoothed[i] = ibi[i] - diff * direction;
		// cout << setw(3) << i << " : " << setprecision(3) << ibi[i] << " -> "
		// 	<< smoothed[i] << " : " << setw(6) << diff << endl;
		mem -= ibi[wrapIndex(i-halfWidth,sz)];
		mem += ibi[wrapIndex(i+halfWidth+1,sz)];
	}
	e = nextNote();
	double spread = parameters->getDouble("random");
	for (int i = 0; i < sz; i++, e = e->nextNote()) {
		double rnd = 2.0 * spread * rand() / (RAND_MAX+1.) - spread;
		e->nextNote()->ev->onset = e->ev->onset + smoothed[i] + rnd;
	}
} // smooth()


// Non-linear filter of salience values to reduce the global peaks
// Not used currently - probably better to find local peaks using peak-picking
void eventList :: normaliseSalience() {
	int count = 0;
	double sum = 0.0;
	double tmp[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	for (eventList* tail = next; tail->ev != NULL; tail = tail->next) {
		sum += tail->ev->salience;
		if (count == 10) {
			sum -= tmp[0];
			for (int i = 0; i < count + 1; i++)
				tmp[i] = tmp[i+1];
		} else
			count++;
		tmp[count-1] = tail->ev->salience;
		tail->ev->salience /= sum / 100.0;
	}
} // normaliseSalience()

eventList* eventList :: nextNote(int eventType) {
	eventList* e = next;
	while ((e->ev != NULL) && (eventType != e->ev->eventType))
		e = e->next;
	return e;
} // nextNote()

// Constants for calculating note/chord salience for beat tracking
#define MERGE_MARGIN 0.070 /* for considering events to be simultaneous */
#define DURATION_WEIGHT 300.0 /* scaling factor for adding duration to volume */
#define PITCH_WEIGHT 4.0 /* scaling factor for adding pitch to volume */

// Sets salience values for all events in the list (see ECAI2000 paper)
eventList* eventList :: calculateMidiSalience() {
	const char* salienceType = parameters->getString("salienceType","additive");
	double mergeMargin = parameters->getDouble("mergeWidth", MERGE_MARGIN);
	double durationWeight, pitchWeight;
	if (strcmp(salienceType, "additive") == 0) {
		durationWeight=parameters->getDouble("durationWeight", DURATION_WEIGHT);
		pitchWeight=parameters->getDouble("pitchWeight", PITCH_WEIGHT);
	}
	double salienceFilter = parameters->getDouble("salienceFilter");
	bool addVelocities =
			!strcmp(parameters->getString("addVelocities","on"),"on");
	eventList* newList = new eventList();
	for (eventList* e = nextNote(); e->ev != NULL; ) {
		double now = e->ev->onset;
		eventList* ptr = e->nextNote();
		event* newEvent = new event(*(e->ev));
		// combine chords
		while ((ptr->ev != NULL) && (ptr->ev->onset - now < mergeMargin)) {
			now += ptr->ev->onset;
			now /= 2.0;
			if (addVelocities)
				newEvent->volume += ptr->ev->volume;	// take sum of volumes
			else if (newEvent->volume < ptr->ev->volume)// else take max volume
			 	newEvent->volume = ptr->ev->volume;
			if (ptr->ev->isBeat())
				newEvent->beat = ptr->ev->beat;
			if (newEvent->offset < ptr->ev->offset)		// take longest note
				newEvent->offset = ptr->ev->offset;
			if (newEvent->pitch > ptr->ev->pitch)		// take lowest pitch
				newEvent->pitch = ptr->ev->pitch;
			ptr = ptr->nextNote();
		}
		if (strcmp(salienceType, "constant") == 0) {
			newEvent->salience = 10.0;
		} else if (strcmp(salienceType, "multiplicative") == 0) {
			newEvent->salience =
				threshold( log( (double) newEvent->volume ), 1.0, 10.0) *
				(newEvent->offset - now) *
				(84.0-threshold((double)newEvent->pitch,48,72));
		} else if (strcmp(salienceType, "additive") == 0) {
			newEvent->salience =
				newEvent->volume +
				// threshold((double)newEvent->volume, 30.0, 90.0) +
				durationWeight * (newEvent->offset - now) +
				pitchWeight * (72.0-threshold((double)newEvent->pitch,48,72));
		} else if (strcmp(salienceType, "bounded") == 0) {
			newEvent->salience = 0.3 * threshold(newEvent->volume,0,200)/200.0 +
					0.2 * (72.0-threshold((double)newEvent->pitch,48,72))/24.0 +
					0.5 * threshold(newEvent->offset - now,0,1);
		}
		newEvent->onset = now;
		if (newEvent->salience > salienceFilter) {
			if (salienceFilter > 0.0)
				newEvent->salience = 10.0;
			newList->add(newEvent);
		} else
			delete(newEvent);
		e = ptr;
	}
	if (parameters->debug("eventBeat", "normalise")) {
		newList->print();
		newList->normaliseSalience();
		newList->print();
	} else if (!strcmp(parameters->getString("normalise","off"), "on"))
		newList->normaliseSalience();
	return newList;
} // calculateMidiSalience()

double eventList :: getBasicTempo(bool isMode) {
	double dummy, mode, mean;
	int idummy;
	getStatistics(INTERPOLATE_ALL, 0, -1, dummy, mode, mean,
					dummy, dummy, idummy, idummy);
	return isMode? mode: mean;
} // getBasicTempo()

// Tries to guess which rhythmic level the beat tracker has used by rounding
//  to the nearest "expected" rhythmic level (1, 1.5, 2, 3, 4 * tempo &inverses)
double eventList :: getRhythmicLevel(eventList* beats) {
	event* first = firstKnown();
	event* last = lastKnown();
	if ((beats->count() < 2) || (first == NULL) || (last == NULL))
		return UNKNOWN;
	double scoreIBI = (last->onset - first->onset) / (last->beat - first->beat);
	double resultIBI = (beats->prev->ev->onset - beats->next->ev->onset) /
						(double)(beats->count() - 1);
	double ratio = scoreIBI / resultIBI;
	double roundedRatio = UNKNOWN;	// indicates error and avoids div by 0 error
	if (     (0.22 < ratio) && (ratio < 0.28))
		roundedRatio = 4.0;
	else if ((0.3 < ratio) && (ratio < 0.367))
		roundedRatio = 3.0;
	else if ((0.4 < ratio) && (ratio < 0.6))
		roundedRatio = 2.0;
	else if ((0.6 < ratio) && (ratio < 0.75))
		roundedRatio = 1.5;
	else if ((0.9 < ratio) && (ratio < 1.3))
		roundedRatio = 1.0;
	else if ((1.35 < ratio) && (ratio < 1.6))
		roundedRatio = 2.0/3.0;
	else if ((1.8 < ratio) && (ratio < 2.5))
		roundedRatio = 0.5;
	else if ((2.7 < ratio) && (ratio < 3.5))
		roundedRatio = 1.0/3.0;
	else if ((3.6 < ratio) && (ratio < 4.6))
		roundedRatio = 0.25;
	else if ((7.6 < ratio) && (ratio < 8.6))
		roundedRatio = 0.125;
	else if (parameters->debug("eventBeat", "getRhythmicLevel")) {
		cerr << setprecision(3)
			 << "Warning: RhythmicLevel " << setw(5) << 1.0 / ratio
			 << " (sc:" << setw(5) << scoreIBI
			 << ";res:" << setw(5) << resultIBI << ")" << endl;
		print();
		beats->print();
	}
	return roundedRatio;
} // getRhythmicLevel()

// Evaluates beat tracking performance; compares tracked events to correct beats
void eventList :: evaluate(eventList* beats) {
	int fp = 0;
	int fn = 0;
	int ok = 0;
	int extra = 0;
	double metricalLevel = getRhythmicLevel(beats);
	eventList* events = makeRealBeatList(metricalLevel, true)->next;
	if (!assertWarning(events->ev != NULL, "No beats in score file"))
		return;
	for (beats = beats->next; (beats->ev != NULL) &&
					(beats->ev->onset < events->ev->onset); beats = beats->next)
		extra++;	// skip any "beats" before music starts
	while ((events->ev != NULL) && (beats->ev != NULL)) {
		if (beats->ev->onset < events->ev->onset) {
			fp++;
			beats = beats->next;
		} else if (beats->ev->onset > events->ev->offset) {
			fn++;
			events = events->next;
		} else {
			ok++;
			beats = beats->next;
			events = events->next;
		}
	}
	while (events->ev != NULL) {
		fn++;
		events = events->next;
	}
	while (beats->ev != NULL) {
		if (events->prev->ev->onset < beats->ev->onset)
			extra++;
		else
			fp++;
		beats = beats->next;
	}
	if (!strcmp(parameters->getString("showStatistics","off"),"on"))
		events->printStatistics();
	if (parameters->getString("matchFile"))
		cout << parameters->getString("matchFile");
	if (metricalLevel == UNKNOWN)
		cout << "-1.0 ";
	else
		cout << setprecision(2) << setw(4) << metricalLevel << " ";
	cout << setprecision(3)
		 << (events->lastKnown()->onset - events->firstKnown()->onset) /
			(events->lastKnown()->beat - events->firstKnown()->beat)
		 << "  " << ((beats == beats->next) ? 0:
			(beats->prev->ev->onset - beats->next->ev->onset) /
			(double)(ok + fp + extra - 1))
		 << "  ok: " << setw(3) << ok
		 << "  f+: " << setw(3) << fp
		 << "  f-: " << setw(3) << fn
		 << "  Score: " << setprecision(1) << setw(5)
		 << (double)(ok*100) / (double)((fp>fn?fp:fn)+ok);
	assertWarning(ok + fp + extra == beats->count(), "Beat count wrong");
	if (! (parameters->debug("eventBeat", "allAgents")))
		cout << endl;
	delete events;
} // evaluate()

// Evaluation of beat tracking a la Cemgil, Kappen, Desain & Honing (JNMR 2001)
// Beat file contains pairs of times representing the first and last event
//  occuring on the beat, or a linear interpolation in the case where no event
//  occurs on the beat. (The file is generated automatically from Honing's
//  score MIDI files using the script ~/honing/scorebeats.sh)
double eventList :: evaluate(const char* beatFile) { // using Honing's formula
	ifstream beats(beatFile);
	const int SZ = 105;
	double min[SZ], max[SZ];
	double err, cumulativeError = 0;
	// double correction = startsWith(beatFile, "michelle")? 1:0;
	int sz = 0;
	while ((sz < SZ) && (beats >> min[sz] >> max[sz])) {
		// min[sz] += correction;		// correct for bug in MMM data
		// max[sz] += correction;
		sz++;
	}
	int i = 0;
	for (eventList* e = next; e->hasMore(); e = e->next) {
		while ((i < sz - 1) && (e->ev->onset > min[i+1]))
			i++;
		if (e->ev->onset > max[i]) {
			if ((i < sz-1) && (min[i+1] - e->ev->onset < e->ev->onset - max[i]))
				err = min[i+1] - e->ev->onset;
			else
				err = e->ev->onset - max[i];
		} else if (e->ev->onset < min[i])
			err = min[i] - e->ev->onset;
		else
			err = 0;
		err = exp(- err * err / 0.0032);
		cumulativeError += err;
		cout << "match " << i << " " << e->ev->onset << " " << err << "\n";
	}
	cumulativeError *= 200.0 / double(sz + count());	// scale to rel. percent
	cout << beatFile << " : "
		 << setprecision(2) << double(sz) / count() << " : "
		 << cumulativeError << endl;
	return cumulativeError;
} // evaluate()

void eventList :: combinePedalTrack(const char* matchFileName) {
	char buf[250];
	makeFileName(matchFileName,
		"/raid/music/data/mozart_by_batik/midi/KV..._SECTIONS/",
		".tmf", buf);	// assume it is already converted to text form
	strncpy(strstr(buf, "..."), strrchr(buf, '/') + 3, 3); // fill in the dots
	eventList* withPedal = (new eventList(buf))->next;
	eventList* original = next;
	while ((original->ev != NULL) && (withPedal->ev != NULL)) {
		assertWarning(fabs(original->ev->onset - withPedal->ev->onset) < 0.0001,
				"File mismatch combining pedal data");
		original->ev->offset = withPedal->ev->offset;	// correct the offsets
									// to account for use of the damper pedal
		original = original->next;
		withPedal = withPedal->next;
	}
	// catch missing events at ends of files
	assertWarning(original->ev == withPedal->ev,
			"Data mismatch at ends of tracks combining pedal data");
	delete withPedal;
} // eventList::combinePedalTrack()

// Beat track a midi file & compare performance with a match file
//void matchFileBeatTrack(char* s) {
//	sprintf(buf,"matchFile=%s%s",s+2, "            "+strlen(s)-12);//closeEyes:)
//	parameters->add(buf);
//	if (beats == NULL) {	// i.e. beat tracking failed
//		printf("%s0.0 0.100  0.500  ok:   0  f+:   0  f-: %3.0lf  Score:   0\n",
//			parameters->getString("matchFile"),
//			events->lastKnown()->beat - events->firstKnown()->beat);
//		return;
//	}
//	if (!strcmp(parameters->getString("showTracking","on"),"on"))
//		tracker->showTracking(events, original->getRhythmicLevel(tracker->events));
//} // matchFileBeatTrack()

// Beat track a midi file; doesn't compare performance with a match file
//  (see Dixon & Cambouropoulos, ECAI 2000)
void midiBeatTrack(const char* s) {
	eventList* original = new eventList(s);
	eventList* salience = original->calculateMidiSalience();
	if (!strcmp(parameters->getString("showSalience","off"), "on"))
		salience->print();
	eventList* beats = salience->beatTrack();
	if (endsWith(s, ".match")) {
		char buf[250];
		makeFileName(s, "matchFile=", NULL, buf);
		strcat(buf, "               ");
		buf[32] = 0;
		parameters->add(buf);
		original->evaluate(beats);
		if (parameters->debug("eventBeat", "allAgents"))
			cout << endl;
	}
	const char* name = parameters->getString("mmmBeats");
	if (name)
		beats->evaluate(name);	// Honing et al. evaluation
	original->addMidiClickTrack(beats);
	name = parameters->getString("midiOut");
	if (name)
		original->writeTMF(name);
	if (!strcmp(parameters->getString("play","on"), "on")) {
		original->play();
	}
	delete beats;
	delete salience;
	delete original;
} // midiBeatTrack()

// Performs beat tracking on a list of musical events
// ... at some point smoothing of results should be added ...
eventList* eventList :: beatTrack() {
	double len = parameters->getDouble("inductionLength", -1.0);
	if (len > 0) {
		parameters->add("debug:eventInduction=IBI");
		for (double start=0; start + len < prev->ev->onset; start += 1.0) {
			eventList* tmp = new eventList();
			tmp->add(this, start, start+len);
			tmp->newInduction();
			delete tmp;
		}
		exit(0);
	}
	agentList* trackers = beatInduction();
	return beatTracking(trackers);
}

eventList* eventList :: beatTracking(agentList* trackers, double restart) {
	trackers->beatTrack(this);
	agent* tracker = trackers->bestAgent();
	if (parameters->debug("eventBeat", "allAgents")) {
		for ( ; trackers->ag != NULL; trackers = trackers->next) {
			trackers->ag->showTracking(this);
			trackers->ag->fillBeats();
			evaluate(trackers->ag->events);
			cout << setprecision(1)
				 << " " << setw(5) << trackers->ag->phaseScore
				 << " " << setw(5) << trackers->ag->tempoScore
				 << " " << setw(5) << (trackers->ag->phaseScore +
					trackers->ag->tempoScore) / (double)trackers->ag->beatCount
				 << endl;
		//  or, instead of using beatCount:
		// * trackers->ag->beatInterval / (prev->ev->onset - next->ev->onset));
		}
	}
	if (tracker == NULL) {
		delete trackers;
		return new eventList();
	}
	if (!strcmp(parameters->getString("showTracking","on"),"on"))
		tracker->showTracking(this);
	tracker->fillBeats(restart);
	eventList* beats = tracker->events;
	tracker->events = NULL;		// so it isn't deleted with the rest
	delete trackers;
	return beats;
} // beatTracking()

eventList* eventList :: beatTrack(eventList* beats, double restart) {
	if (!assertWarning(beats != NULL, "Crash avoided here"))
		beats = new eventList();
	if (restart >= 0) {
		while (beats->next->ev != NULL)
			if (beats->next->ev->onset > restart)
				beats->remove(beats->next->ev);
			else
				beats = beats->next;
		beats = beats->next; 		// set back to head
	} else if (beats->prev->ev != NULL)
		restart = beats->prev->ev->onset;
	int size = beats->count();
	agentList* a;
	if (size < 2) {
		a = beatInduction();
		if (size == 1)
			for (agentList* ptr = a; ptr->ag != NULL; ptr = ptr->next) {
				ptr->ag->beatTime = beats->prev->ev->onset;
				ptr->ag->beatCount = 1;
				ptr->ag->phaseScore = 1.0;
				ptr->ag->events->add(beats->prev->ev);
			}
	} else {
		a = new agentList();
		a->add(new agent(
			(beats->prev->ev->onset - beats->next->ev->onset)/double(size-1)));
		a->ag->beatTime = beats->prev->ev->onset;
		a->ag->beatCount = size;
		a->ag->phaseScore = 1.0;
		a->ag->events->add(beats);
		if (parameters->debug("eventBeat", "tempo"))
			a->ag->print();
	}
	return beatTracking(a, restart);
} // beatTrack() -- restart from previous attempt

void showDistributions(char* s) {
	eventList* original = new eventList(s); // performance data - pedal + score
	double windowSize = parameters->getDouble("windowSize", 5.0);
	double windowSlide = parameters->getDouble("windowSlide", 1.0);
	gsub(s,'_','-');	// Matlab uses '_' as a formatting command
	cout << "name = '" << ((s==strstr(s,"m/"))? s+2: s) << "';" << endl;
	double rhythmicLevels[7];
	int numLevels = 0;
	int top = parameters->getInt("beatsPerBar");
	switch (top) {
		case 0:	// assume 4/4 if not specified
		case 4:
			rhythmicLevels[numLevels++] = 4.0;
		case 2:
			rhythmicLevels[numLevels++] = 2.0;
			break;
		case 12:
			rhythmicLevels[numLevels++] = 12.0;
		case 6:
			rhythmicLevels[numLevels++] = 6.0;
		case 3:
			rhythmicLevels[numLevels++] = 3.0;
			break;
		case 9:
			rhythmicLevels[numLevels++] = 9.0;
			rhythmicLevels[numLevels++] = 3.0;
			break;
		default:
			assertWarning(FALSE, "Time signature not recognised");
			rhythmicLevels[numLevels++] = double(top);
			break;
	}
	rhythmicLevels[numLevels++] = 1.0;
	const char* levels = parameters->getString("beatSubdivisions", "[]");
	while (*(++levels) != ']') {
		rhythmicLevels[numLevels++] = 1.0 / atof(levels);
		while (*(++levels) != ',')
			if (*levels == ']') {
				levels--;
				break;
			}
	}
	eventList* e;
	for (int i = numLevels - 1; i >= 0; i--) {
		e = original->makeRealBeatList(rhythmicLevels[i], true);
		e->matlabPrint("vals", INTERPOLATE_ONCE);
		if (i == numLevels - 1) {
			cout << "ioi = zeros(" << numLevels << ",length(vals),2);\n"
				 << "vlengths = zeros(" << numLevels << ", 1);\n"
				 << "levels = zeros(" << numLevels << ", 1);\n"
				 << "stats = zeros(" << numLevels << ", 8);\n";
		}
		cout << "ioi(" << numLevels - i << ",1:length(vals),:) = vals;\n"
			 << "vlengths(" << numLevels - i << ") = length(vals);\n"
			 << "levels(" << numLevels - i << ") = "
			 << setprecision(2) << rhythmicLevels[i] << ";\n"
			 << "stats(" << numLevels - i << ",:) = [\n";
		// for (double t = e->next->ev->onset;
		// 			t + windowSize < e->prev->ev->onset; t +=windowSlide)
			e->printStatistics(INTERPOLATE_ONCE, 0, -1);
			// e->printStatistics(INTERPOLATE_ONCE, t, t + windowSize);
		cout << "];" << endl;
	}
	cout << "setFlag(0);\nwhile (getFlag==0)\n"
			"\tmultiDist(ioi, name, levels, vlengths);\n"
			"\tbuttons('Hist');\n\tif (getFlag == 0)\n"
			"\t\tmultiHist(ioi, name, levels, vlengths, stats);\n"
			"\t\tbuttons;\n\tend\nend\ncount=count+getFlag;\n" << endl;
} // showDistributions()

void showDistributions1(char* s) {
	// parameters->add("format=match");
	double rhythmicLevels[7] = {0.25,0.333333333333333333,0.5,1.0,2.0,3.0,4.0};
	eventList* original = new eventList(s);	// performance data - pedal + score
	gsub(s,'_','-');
	cout << "name = '" << (strncmp(s,"m/",2)? s: s+2) << "';" << endl;
	eventList* e;
	for (int i=0; i<7; i++) {
		cout << "level=" << setprecision(2) << rhythmicLevels[i] << "; ";
		e = original->makeRealBeatList(rhythmicLevels[i], true);
		cout << "[mode mean sd rd n n1 ad] = [\n";
		e->printStatistics(INTERPOLATE_ONCE);
		cout << "];\nif (n>0) tempoDist(ioi,name,level,4,mode,mean); pause; end"
			 << endl;
	}
} // showDistributions1()
