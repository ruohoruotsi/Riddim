//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: event.cpp
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

#include "event.h"
#include "includes.h"
#include "param.h"
#include "playq.h"
#include "tmf.h"
#include "util.h"


// implementation: CLASS EVENT

event :: event(double on, double off, int p, int vol, double bt) {
	init(NOTE_ON, on, off, off, p, vol, 0, -1, 0, 0, bt);
}

event :: event(int tp, double on, int p, int vel, int vox, int trk, int ch) {
	init(tp, on, on, on, p, vel, vox, trk, ch, 0, UNKNOWN);
}

event :: event(const event& e) {
	init(e.eventType, e.onset, e.noteOff, e.offset, e.pitch, e.volume,
		 e.voice, e.track, e.channel, e.flags, e.beat);
}

event :: ~event() {
	delete data;
}

// shared code for constructors
void event :: init(int tp, double on, double off, double eOff, int p, int vol,
					int vox, int trk, int ch, int flg, double bt) {
	eventType = tp;
	onset = on;
	noteOff = off;
	offset = eOff;
	pitch = p;
	volume = vol;
	voice = vox;
	track = trk;
	channel = ch;
	flags = flg;
	beat = bt;
	salience = 0.0;
	data = NULL;
	references = 0;
} // init()

bool event :: isBeat(double level) const {
	return fabs(beat / level - rint(beat / level)) < 0.001;
} // isBeat()

int event :: match(event* e, double errorWindow) {
	if ((e == NULL) || e->isMatched())
		return FALSE;
	double timeDiff = fabs(e->onset - onset);
	if (e->pitch == pitch) {
		setMatched();
		e->setMatched();
		if (timeDiff < errorWindow)
			return MATCHED_FLAG;
		else
			return TIME_ERROR;
	} else {
		double ratio = midiFrequency(e->pitch) / midiFrequency(pitch);
		if (fabs(ratio - rint(ratio)) < 0.001)
			return int(ratio);
		ratio = 1.0 / ratio;
		if (fabs(ratio - rint(ratio)) < 0.001)
			return int(rint(-ratio));
	}
	return FALSE;
} // match()

void event :: print(bool shortFormat) const {
	if (shortFormat)
		cout << "n = " << pitch << " : " << setprecision(3) << setw(7) << onset
			 << " - " << setw(7) << offset << " : v = " << setw(3) << volume;
	else {
		cout << "event.(on=" << setprecision(3) << setw(7) << onset << "; "
			 << "off=" << setw(7) << offset << "; "
			 << "note=" << setw(3) << pitch << "; "
			 << "v=" << setw(3) << volume << "; "
			 << "vx=" << setw(2) << voice << "; "
			 << "sal=" << setw(6) << setprecision(2) << salience << "; "
			 << "beat=" << setw(6) << (beat!=UNKNOWN?beat:-99.99) << ")" <<endl;
	}
	if (parameters->debug("event", "references"))
		cout << "\treferences=" << references << endl;
} // print()


// implementation: CLASS EVENTLIST

eventList :: eventList() {
	init(NULL, this, this);	// end marker
}

eventList :: eventList(event* newEvent, eventList* p, eventList* n) {
	init(newEvent, p, n);
}

eventList :: eventList(eventList* clone) {
	init(NULL, this, this);
	add(clone);
}

eventList :: ~eventList() {
	while (next->ev != NULL) {	// recursive version caused stack overflow
		if (!--next->ev->references) {
			if (parameters->debug("event", "memoryManagement")) {
				cout << "Deleting: ";
				next->ev->print();
			}
			delete next->ev;
		}
		next->ev = NULL;
		eventList* tmp = next;
		next = next->next;
		tmp->next = tmp;
		delete tmp;
	}
} // eventList destructor

void eventList :: init(event* newEvent, eventList* p, eventList* n) {
	ev = newEvent;
	if (newEvent != NULL)
		newEvent->references++;
	prev = p;
	next = n;
} // init()

void eventList :: add(event* newEvent) {
	prev->next = new eventList(newEvent, prev, this);
	prev = prev->next;
} // add(event*)

void eventList :: add(eventList* el, double start, double end) {
	for (el = el->next; el->ev != NULL; el = el->next)
		if (((start < 0) || (start <= el->ev->onset)) &&
				((end < 0) || (end >= el->ev->onset)))
			add(el->ev);
} // add(eventList*)

void eventList :: insert(event* newEvent) {
	eventList* ptr = next;
	while ((ptr->ev != NULL) && (newEvent->onset > ptr->ev->onset))
		ptr = ptr->next;
	ptr->add(newEvent);
} // insert()

void eventList :: replace(eventList* newList) { // swaps heads of lists
	eventList* tmp;								//  and then deletes newList
	tmp = next;									//  (i.e. prev contents of this)
	next = newList->next;
	newList->next = tmp;
	tmp = prev;
	prev = newList->prev;
	newList->prev = tmp;
	next->prev = this;
	prev->next = this;
	newList->next->prev = newList;
	newList->prev->next = newList;
	delete newList;
} // replace()

void eventList :: remove() {
	prev->next = next;
	next->prev = prev;
	if (!--ev->references)
		delete ev;
	ev = NULL;
	prev = next = this;
} // remove()

void eventList :: remove(event* oldEvent) {
	if (!assertWarning(oldEvent != NULL, "eventList::remove(): NULL event"))
		return;
	for (eventList* e = next; e->ev != NULL; e = e->next) {
		if (e->ev == oldEvent) {
			e->remove();
			delete(e);
			return;
		}
	}
	assertWarning(false, "eventList::remove(): event not found");
} // remove(event*)

// Clears events matching eType between start and start+len
// Default len=-1 -> to end of events
// Default eType=0 -> all events
void eventList :: clear(double start, double len, int eType) {
	for (eventList* e = next; e->ev != NULL; ) {
		if ((e->ev->onset < start) || ((eType>0) && (eType!=e->ev->eventType)))
			e = e->next;
		else if ((len < 0) || (e->ev->onset <= start + len)) {
			eventList* tmp = e;
			e = e->next;
			tmp->remove();
			delete(tmp);
		} else
			break;
	}
} // clear()

eventList* eventList :: head() {
	eventList* current = this;
	while (current->ev != NULL)
		current = current->next;
	return current;
} // head()

// Finds the next event after time
eventList* eventList :: nextEvent(double time) {
	eventList* current = this;
	while ((current->ev != NULL) && (current->ev->onset < time))
		current = current->next;
	while ((current->prev->ev != NULL) && (current->prev->ev->onset > time))
		current = current->prev;
	return current;
} // nextEvent()

eventList* eventList :: nearest(double time) { // finds nearest event to time
	if (prev == next)	// 0 or 1 element in list 
		return prev;
	eventList *current = nextEvent(time);
	if (current->ev == NULL)
		return current->prev;
	if (current->prev->ev == NULL)
		return current;
	if (fabs(current->ev->onset - time) < fabs(current->prev->ev->onset - time))
		return current;
	else
		return current->prev;
} // nearest()

int eventList :: count(int eventType) const {
	int counter = 0;
	for (eventList* e = next; e->ev != NULL; e = e->next)
		if ((eventType < 0) || (eventType == e->ev->eventType))
			counter++;
	return counter;
} // count()

void eventList ::
printStatistics(int interpolate, double start, double stop) const {
	double mean, mode, median, sd, ad;
	int n, n1;
	getStatistics(interpolate, start, stop, median, mode, mean, sd, ad, n, n1);
	cout << setprecision(3)
		 << setw(7) << median << " "
		 << setw(5) << mode << " "
		 << setw(5) << mean << " "
		 << setw(5) << sd << " "
		 << setw(5) << sd / mean << " "
		 << setw(3) << n << " "
		 << setw(3) << n1 << " "
		 << setw(5) << ad << endl;
} // printStatistics()

void eventList :: getStatistics(int interpolate, double start, double stop,
		double& oMedian, double& oMode, double& oMean, double& oSd,
		double& oAd, int& oN, int& oN1) const {
	double x, sx, sxx, mean, sd, prevx, prevOnset, absdev, ioi[count()];
	int n, n1;
	if ((stop < 0) && (prev->ev != NULL))
		stop = prev->ev->onset;
	n = n1 = 0;
	sx = sxx = absdev = 0.0;
	prevx = prevOnset = -1.0;
	for (const eventList* e = ((eventList*)this)->nextEvent(start)->next;
				(e->ev != NULL) && (e->ev->onset <= stop); e = e->next) {
		if (	(interpolate & INTERPOLATE_ALL) ||
				((interpolate & INTERPOLATE_ONCE) &&
					((e->prev->ev == NULL) || (e->prev->ev->pitch > 0))) ||
				((interpolate & BEATS_ONLY) && e->ev->isBeat()) ||
				((interpolate & INTERPOLATE_NONE) &&
					((e->prev->ev == NULL) || (e->prev->ev->pitch > 0)) &&
					(e->ev->pitch > 0))) {
			if (prevOnset < 0) {
				prevOnset = e->ev->onset;
				continue;
			}
			if (interpolate & BEATS_ONLY) {
				x = e->ev->onset - prevOnset;
				prevOnset = e->ev->onset;
			} else
				x = e->ev->onset - e->prev->ev->onset;
			sx += x;
			sxx += x * x;
			ioi[n++] = x;
			if (prevx >= 0)
				absdev += fabs((x - prevx) / (prevx + x) * 2);
			prevx = x;
			if ((e->prev->ev->pitch>0) && (e->ev->pitch > 0))
				n1++;
		}
	}
	if (n > 0) {
		mean = sx / double(n);
		sd = sxx / double(n) - mean * mean;		// variance, not sd yet
		if (sd < 0.0)	// rounding error when all x's are equal
			sd = 0.0;
		else
			sd = sqrt(sd);		// standard deviation
		quickSort(ioi,n);
		double modeWindow = parameters->getDouble("modeWindow",0.05);
		// printf("%7.3lf %5.3lf %5.3lf %5.3lf %5.3lf %3d %3d %5.3lf\n",
		// 		(start+stop)/2, findMode(ioi,n,modeWindow), mean, sd, sd / mean,
		// 		n, n1, n > 1? absdev / double(n-1): 0);
		oMedian = ioi[n/2];
		oMode = findMode(ioi,n,modeWindow);
		oMean = mean;
		oSd = sd;
		oAd = n > 1? absdev / double(n-1): 0;
		oN = n;
		oN1 = n1;
	}
} // getStatistics()

void eventList :: matlabPrint(const char* name, int interp,
							  double start,double stop) const {
	double x = -1.0;
	double corrOffset = parameters->getDouble("matlabOffset", 0);
	double corrFactor = parameters->getDouble("matlabFactor", 15.0/44100.0);
	cout.setf(ios::showpoint);
	cout.setf(ios::fixed);
	cout << setprecision(4) << name << " = [" << endl;
	for (const eventList* e = ((eventList*)this)->nextEvent(start);
				(e->ev != NULL) && ((stop < 0) || (e->ev->onset <= stop));
				e = e->next) {
		if ((interp & INTERPOLATE_ALL) ||
				((interp & INTERPOLATE_ONCE) &&
					((e->prev->ev == NULL) || (e->prev->ev->pitch > 0))) ||
				(((e->prev->ev == NULL) || (e->prev->ev->pitch > 0)) &&
					(e->ev->pitch > 0))) {
			if ((x < 0) && (corrOffset == 0)) {
				x = 0;
				continue;
			}
			cout << setw(8) << ((e->ev->beat > 0) ? e->ev->beat : 0);
			if (corrOffset != 0)
				x = e->ev->onset + corrOffset;
			else
				x = e->ev->onset - e->prev->ev->onset;
			// cout << setw(8) << x;
			x *= 1 + corrFactor;
			cout << setw(8) << x << endl;
		}
	}
	cout << "];" << endl;
} // matlabPrint()

void eventList :: print() const {
	cout << "eventList.(  " << count() << " items" << endl;
	for (eventList* p = next; p->ev != NULL; p = p->next)
		p->ev->print();
	cout << ")" << endl;
} // print()

void eventList :: reversePrint() const {
	cout << "eventList.( (reversed) " << endl;
	for (eventList* p = prev; p->ev != NULL; p = p->prev)
		p->ev->print();
	cout << ")" << endl;
} // reversePrint()

// Creates TMF file and MIDI file in /tmp and queues the play request
void eventList :: play(double start, double end) {
	static int count = 0;
	// writeTMF("/tmp/play.tmf", start, end);
	// ostrstream command;
	ostrstream filename;
	filename << "/tmp/play" << count++ << ".mid" << ends;
	oMIDI dummy(filename.str(), this, start, end);
	// command << parameters->getString("t2m", "/raid/music/bin/t2mf")
	// 		<< " /tmp/play.tmf " << filename.str() << " >/dev/null" << ends;
	// system(command.str());
	playq->queueMidi(filename.str());
	// command.freeze(0);
	filename.freeze(0);	// unfreeze streams so that memory is de-allocated
} // play()

// implementation: CLASS IOILIST

ioiList :: ioiList(event* s, event* e) {
	start = s;
	end = e;
	next = NULL;
	interval = e->onset - s->onset;
	cluster = -1;
}

ioiList :: ~ioiList() {
	if (next != NULL)
		delete next;
}

void ioiList :: print() const {
	cout << "ioiList.(" << endl;
	for (const ioiList* p = this; p != NULL; p = p->next)
		cout << "\tinterval = " << setprecision(3) << p->interval
			 << "; cluster = " << p->cluster << endl;
	cout << ")" << endl;
}


// implementation: CLASS CLUSTER

cluster :: cluster() {
	init();
}

cluster :: cluster(ioiList* ioi) {
	init();
	add(ioi);
}

void cluster :: init() {
	size = 0;
	sum = 0;
	intervals = NULL;
}

cluster :: ~cluster() {
	delete intervals;
}

void cluster :: add(ioiList* ioi) {
	while (ioi != NULL) {
		ioiList* thisIOI = ioi;
		ioi = ioi->next;
		size++;
		sum += thisIOI->interval;
		thisIOI->next = intervals;	// place at head of list
		intervals = thisIOI;
	}
}

void cluster :: add(cluster* c) {
	add(c->intervals);
	c->intervals = NULL;
	delete c;
}

double cluster :: value() const {
	if (size == 0)
		return 0.0;
	else
		return sum / double(size);
}

void cluster :: print() const {
	cout << "cluster.(sum=" << setprecision(3) << sum << "; size="
		 << size << "; average=" << value() << "; score=" << score << endl;
	intervals->print();
	cout << ")" << endl;
}
