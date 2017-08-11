//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: main2.cpp
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
#include "gui.h"
#include "param.h"
#include "playq.h"
#include "realTime.h"
#include "sample.h"
#include "spectrum.h"
#include "util.h"


parameterList* parameters;
gui* jgui;
playQ* playq;


int main(int argc, char** argv) {
    char* optarg;
	char option;
	int optind;
	char c;

	parameters = new parameterList();
	jgui = NULL;
	playq = new playQ();
	parameters->readDefaults();
	parameters->write();
	cout.setf(ios::fixed | ios::showpoint, ios::floatfield | ios::scientific);
	cerr.setf(ios::fixed | ios::showpoint, ios::floatfield | ios::scientific);
	if (parameters->debug("main", "showOptions"))
		for (int i=0; i<argc; i++)
			cout << argv[i] << (i==argc-1?'\n':' ');
	for (int i=0; i<argc; i++)
		cerr << argv[i] << (i==argc-1?'\n':' ');
	for (optind = 1; optind < argc; optind++) {
		if (!assertWarning(argv[optind][0] == '-', "Illegal option format"))
			continue;
		option = argv[optind][1];
		if (!assertWarning(option != 0, "Illegal option format"))
			continue;
		if (strchr("gx", option))		// options without arguments
			optarg = NULL;
		else if (argv[optind][2])		// Is the arg in this or next argv[i]?
			optarg = argv[optind] + 2;
		else
			optarg = argv[++optind];
		switch (option) {
			case 'a': 	// beat tracking of audio file
				audioBeatTrack(optarg);
				break;
			case 'A': { // adding audio + beats
				sample s(optarg);
				char* btFile = parameters->getString("beats");
				sample* t = &s;
				if (btFile) {
					eventList e(btFile);
					t = s.addClickTrack(&e);
				}
				double pad = parameters->getDouble("padding");
				if (pad > 0) {
					sample space(int(pad * t->rate()), t->rate(),t->channels());
					t->prepend(space);
					t->append(space);
				}
				t->writeFile(parameters->getString("audioOut", "tmp.wav"));
				if (!strcmp(parameters->getString("play", "on"), "on"))
					t->play();
				if (t != &s)
					delete t;
				break;
			}
			case 'b': { // mix
				parameters->add("channels=both");		// reverse default
				sample s(optarg);
				s.mix(parameters->getDouble("bias", 0.5));
				if (!strcmp(parameters->getString("play", "on"), "on"))
					s.play();
				s.writeFile(parameters->getString("audioOut", "mix.wav"));
				break;
			}
			case 'B': { // remix
				parameters->add("channels=both");		// reverse default
				sample s(optarg);
				s.remix();
				if (!strcmp(parameters->getString("play", "on"), "on"))
					s.play();
				s.writeFile(parameters->getString("audioOut", "remix.wav"));
				break;
			}
			case 'c':	// convert file: wav <-> snd or fade in/out or cut
				if (char* name = parameters->getString("audioOut")) {
					if (parameters->find("channels") < 0)
						parameters->add("channels=both");	// reverse default
					sample s(optarg);
					s.writeFile(name);
				}
				break;
			case 'C': {	// correct a tmf file to match the audio data
				eventList* e = new eventList(optarg);
				double factor = parameters->getDouble("factor", 44115./44100.);
				double offs = parameters->getDouble("offset");
				if (e->next->hasMore())
					offs -= e->nextNote()->ev->onset * factor;
				for (e = e->next; e->hasMore(); e = e->next) {
					e->ev->onset = e->ev->onset * factor + offs;
					if (e->ev->onset < 0)
						e->ev->onset = 0;
					e->ev->offset = e->ev->offset * factor + offs;
					e->ev->noteOff = e->ev->noteOff * factor + offs;
				}
				e->writeTMF(parameters->getString("tmfOut", "correct.tmf"));
				delete e;
				break;
			}
			case 'g':	// beat tracking GUI
				jgui = new gui();
				break;
			case 'm':	// beat tracking of midi file
			case 'M':	// beat tracking of match file
				midiBeatTrack(optarg);
				break;
			case 'O': {	// test onset detection
				sample s(optarg);
				eventList* e = s.getOnsets();
				e->writeTMF(parameters->getString("tmfOut", "onsets.tmf"));
				delete e;
				break;
			}
			case 'p':	// beatmix a list of previously annotated songs
				playSongList(argv+optind);
				optind = argc;	// done
				break;
			case 'P': {	// specify parameter file
				parameters->clear();
				ifstream p(optarg);
				parameters->read(p);
				parameters->write();
				break;
			}
			case 'S': {	// smooth a beat file
				eventList e(optarg);
				e.smooth(parameters->getInt("smooth", 1));
				if (char* name = parameters->getString("tmfOut"))
					e.writeTMF(name);
				if (!strcmp(parameters->getString("play", "on"), "on"))
					e.play();
				break;
			}
			case 'x':	// suppress play()
				parameters->add("play=off");
				break;
			case 'y': {		// label score beats (performance listening exp.)
				eventList* data = new eventList(optarg);
				double level = parameters->getDouble("level", 1.0);
				eventList* beatPositions = data->makeRealBeatList(level, true);
				data->addMidiClickTrack(beatPositions);
				const char* name = parameters->getString("midiOut");
				if (name)
					data->writeTMF(name);
				name = parameters->getString("beatsOut");
				if (name)
					beatPositions->writeTMF(name);
				name = parameters->getString("matlabOut");
				if (name)
					beatPositions->matlabPrint(name);
				if (!strcmp(parameters->getString("play", "on"), "on"))
					data->play();
				break;
			}
			case '-':	// global parameter setting
				parameters->add(optarg);
				break;
			default:
				assertWarning(FALSE, "Illegal option format");
				break;
		}
	}
	if ((jgui == NULL) && (argc == 1))	// endsWith(argv[0], "beatroot"))
		jgui = new gui();
	if (jgui != NULL) {
		cerr << "Main finished: waiting for GUI\n";
		jgui->processRequests();
		delete jgui;	// destructor
	}
	cerr << "Main finished: will wait for audio thread if running\n";
	delete playq; // destructor waits for thread to end
} // main()
