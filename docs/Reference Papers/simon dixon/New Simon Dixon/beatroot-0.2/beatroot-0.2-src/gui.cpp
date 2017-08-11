//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: gui.cpp
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
#include "local.h"
#include "param.h"
#include "playq.h"
#include "sample.h"
#include "spectrum.h"
#include "util.h"

#ifdef _WIN32
#define PATH_SEPARATOR ";"
#else /* UNIX */
#define PATH_SEPARATOR ":"
#endif

#define USER_CLASSPATH "/raid/user/simon/beatmix/src" /* find mine first */
#define GUI_CLASSPATH "/raid/music/bin/classes" /* where Gui is */


gui :: gui() {
    JDK1_1InitArgs vm_args;
    jclass cls;
    jmethodID mid;
    jstring jstr;
    jobjectArray args;

	audioData = NULL;
	audioPlayData = NULL;
	spectroData = NULL;
	audioEnv = NULL;
	midiData = NULL;
	beatData = NULL;
	undoData = NULL;
    vm_args.version = 0x00010001;	// this seems to work with 1.2
    JNI_GetDefaultJavaVMInitArgs(&vm_args);

    // add classpath of Gui code
    string classpath(vm_args.classpath);
	if (char* userClassPath = parameters->getString("classpath"))
		classpath = classpath + PATH_SEPARATOR + userClassPath;
	else
		classpath = classpath + PATH_SEPARATOR + USER_CLASSPATH
							  + PATH_SEPARATOR + GUI_CLASSPATH;
    vm_args.classpath = charArrayCopy(classpath.c_str());

    jAssert(JNI_CreateJavaVM(&jvm,(void**)&env,&vm_args) >= 0,
						"Could not create Java virtual machine", true);
    cls = env->FindClass("Gui");
    jAssert(cls != NULL, "Can't find Gui class", true);
    mid = env->GetMethodID(cls, "<init>", "([Ljava/lang/String;)V");
    jAssert(mid != NULL, "Can't find Gui.<init>", true);
    jstr = env->NewStringUTF(parameters->getString("javaFlags", ""));
    jAssert(jstr != NULL, "Out of memory", true);
    args = env->NewObjectArray(1, env->FindClass("java/lang/String"),jstr);
    jAssert(args != NULL, "Out of memory", true);
	jGUI = env->NewObject(cls, mid, args);
	jAssert(jGUI != NULL, "Can't create Gui object", true);
    jGetBeatData = env->GetMethodID(cls, "getBeatData", "()[D");
    jAssert(jGetBeatData != NULL, "Can't find Gui.getBeatData()", true);
    jSetAudioData = env->GetMethodID(cls, "setAudioData", "([D[D[I)V");
    jAssert(jSetAudioData != NULL, "Can't find Gui.setAudioData()", true);
    jSetBeatData = env->GetMethodID(cls, "setBeatData", "([D)V");
    jAssert(jSetBeatData != NULL, "Can't find Gui.setBeatData()", true);
    jSetSpectroData = env->GetMethodID(cls, "setSpectroData", "([DIDDDD)V");
    jAssert(jSetSpectroData != NULL, "Can't find Gui.setSpectroData()", true);
    jSetMidiData = env->GetMethodID(cls, "setMidiData", "([D[D[I)V");
    jAssert(jSetMidiData != NULL, "Can't find Gui.setMidiData()", true);
    jGetRequest = env->GetMethodID(cls, "getRequest", "()Ljava/lang/String;");
    jAssert(jGetRequest != NULL, "Can't find Gui.getRequest()", true);
	jShowMessage = env->GetMethodID(cls, "showMessage","(Ljava/lang/String;)V");
    jAssert(jShowMessage != NULL, "Can't find Gui.showMessage()", true);
	jSetBlindMode = env->GetMethodID(cls, "setBlindMode","(II)V");
    jAssert(jSetBlindMode != NULL, "Can't find Gui.setBlindMode()", true);
	mode = parameters->getInt("blindMode", SHOW_SPECTRO);
	setMode(0, mode);
} // gui constructor

gui :: ~gui() {
	if (jvm != NULL)
		jvm->DestroyJavaVM();	// doesn't return - exit()'s
} // gui destructor

// Shows JVM error message if a Java exception occurred
int gui :: jAssert(int check, const char* mesg, bool fatal) {
	if (env->ExceptionOccurred()) {
		env->ExceptionDescribe();
		check = 0;
	}
	if (fatal)
		asserts(check, mesg);
	else
		assertWarning(check, mesg);
	return check;
} // gui :: jAssert()

void gui :: setMode(int on, int off) {
	env->CallVoidMethod(jGUI, jSetBlindMode, (jint) on, (jint) off);
	jAssert(TRUE, "Calling Java method Gui.setBlindMode()");
} // setMode()

void gui :: rememberBeatData() {
	if (beatData != NULL) {
		if (undoData != NULL)
			delete undoData;
		undoData = new eventList(beatData);
	}
} // gui :: rememberBeatData()

void gui :: restoreBeatData() {
	if (undoData != NULL) {
		eventList* tmp = undoData;
		if (beatData != NULL)
			undoData = beatData;
		beatData = tmp;
	}
} // gui :: restoreBeatData()


void gui :: updateBeatData() {
	if (beatData != NULL)
		delete beatData;
	beatData = getBeatData();
	if (audioData != NULL) {
		if (!strcmp(parameters->getString("stereo","off"), "on")) {
			if (audioPlayData != NULL)
				audioPlayData = audioPlayData->addClickTrack(beatData);
			else
				audioPlayData = audioData->addClickTrack(beatData);
		} else {
			if ((audioPlayData != NULL) && (audioPlayData != audioData))
				delete audioPlayData;
			audioPlayData = new sample(*audioData);
			audioPlayData->overlayClickTrack(beatData);
		}
	} else if (midiData != NULL) {
		midiData->addMidiClickTrack(beatData);
	}
} // updateBeatData()

eventList* gui :: getBeatData() {
	eventList* e = new eventList();
	jdoubleArray jbeats=(jdoubleArray)env->CallObjectMethod(jGUI, jGetBeatData);
	if (!jAssert(jbeats != NULL, "Calling Java method Gui.getBeatData()"))
		return e;
	jdouble* beats = env->GetDoubleArrayElements(jbeats, NULL);
	if (!jAssert(beats != NULL, "Getting Java data"))
		return e;
	jint beatCount = env->GetArrayLength(jbeats);
	if (!jAssert(beatCount >= 0, "Getting Java array length"))
		return e;
	e->addMidiClickTrack(beats, beatCount);
	env->ReleaseDoubleArrayElements(jbeats, beats, 0);
	return e;
} // gui :: getBeatData()

void gui :: updateBeatDisplay() {
	if (beatData == NULL)
		return;
	jint beatCount = beatData->count(NOTE_ON);
	jdouble beats[beatCount];
	for (int i = 0; i < beatCount; i++) {
		beatData = beatData->nextNote();
		beats[i] = (jdouble) beatData->ev->onset;
	}
	beatData = beatData->head();
	jdoubleArray jbeats = env->NewDoubleArray(beatCount);
	if (!jAssert(jbeats != NULL, "Can't create array jbeats"))
		return;
	env->SetDoubleArrayRegion(jbeats, 0, beatCount, beats);
	if (!jAssert(TRUE, "Initialising Java array region jbeats"))
		return;
	env->CallVoidMethod(jGUI, jSetBeatData, jbeats);
	jAssert(TRUE, "Calling Java method Gui.setBeatData()");
} // gui :: updateBeatDisplay()

void gui :: updateMidiDisplay() {
	jint eventCount = midiData->count(NOTE_ON);
	jdouble onsets[eventCount];
	jdouble offsets[eventCount];
	jint pitches[eventCount];
	for (int i = 0; i < eventCount; i++) {
		midiData = midiData->nextNote();
		onsets[i] = (jdouble) midiData->ev->onset;
		offsets[i] = (jdouble) midiData->ev->offset;
		pitches[i] = (jint) midiData->ev->pitch;
	}
	midiData = midiData->head();
	jdoubleArray jonsets = env->NewDoubleArray(eventCount);
	if (!jAssert(jonsets != NULL, "Can't create array jonsets"))
		return;
	env->SetDoubleArrayRegion(jonsets, 0, eventCount, onsets);
	if (!jAssert(TRUE, "Initialising Java array region jonsets"))
		return;
	jintArray jpitches = env->NewIntArray(eventCount);
	if (!jAssert(jpitches != NULL, "Can't create array jpitches"))
		return;
	env->SetIntArrayRegion(jpitches, 0, eventCount, pitches);
	if (!jAssert(TRUE, "Initialising Java array region jpitches"))
		return;
	jdoubleArray joffsets = env->NewDoubleArray(eventCount);
	if (!jAssert(joffsets != NULL, "Can't create array joffsets"))
		return;
	env->SetDoubleArrayRegion(joffsets, 0, eventCount, offsets);
	if (!jAssert(TRUE, "Initialising Java array region joffsets"))
		return;
	env->CallVoidMethod(jGUI, jSetMidiData, jonsets, joffsets, jpitches);
	jAssert(TRUE, "Calling Java method Gui.setMidiData()");
} // gui :: updateMidiDisplay()

void gui :: updateAudioDisplay() {
	jint onsetCount = midiData->count();
	jdouble onsets[onsetCount];
	jint envCount = audioEnv->count();
	jdouble envTimes[envCount];
	jint envMagnitudes[envCount];
	for (int i = 0; i < onsetCount; i++) {
		midiData = midiData->next;
		onsets[i] = (jdouble) midiData->ev->onset;
	}
	midiData = midiData->next;
	for (int i = 0; i < envCount; i++) {
		audioEnv = audioEnv->next;
		envTimes[i] = (jdouble) (audioEnv->ev->onset);
		envMagnitudes[i] = (jint) rint(audioEnv->ev->salience);
	}
	audioEnv = audioEnv->next;
	jdoubleArray jonsets = env->NewDoubleArray(onsetCount);
	if (!jAssert(jonsets != NULL, "Can't create array jonsets"))
		return;
	env->SetDoubleArrayRegion(jonsets, 0, onsetCount, onsets);
	if (!jAssert(TRUE, "Initialising Java array region jonsets"))
		return;
	jdoubleArray jenvTimes = env->NewDoubleArray(envCount);
	if (!jAssert(jenvTimes != NULL, "Can't create array jenvTimes"))
		return;
	env->SetDoubleArrayRegion(jenvTimes, 0, envCount, envTimes);
	if (!jAssert(TRUE, "Initialising Java array region jenvTimes"))
		return;
	jintArray jenvMagnitudes = env->NewIntArray(envCount);
	if (!jAssert(jenvMagnitudes != NULL, "Can't create array jenvMagnitudes"))
		return;
	env->SetIntArrayRegion(jenvMagnitudes, 0, envCount, envMagnitudes);
	if (!jAssert(TRUE, "Initialising Java array region jenvMagnitudes"))
		return;
	env->CallVoidMethod(jGUI, jSetAudioData, jonsets, jenvTimes,jenvMagnitudes);
	jAssert(TRUE, "Calling Java method Gui.setAudioData()");
	if (spectroData != NULL) {
		int spectroSize = spectroData->getSizeF() * spectroData->getSizeT();
		jdoubleArray jSpectro = env->NewDoubleArray(spectroSize);
		if (!jAssert(jSpectro != NULL, "Can't create array jSpectro"))
			return;
		env->SetDoubleArrayRegion(jSpectro, 0,
								  spectroSize, spectroData->getData());
		env->CallVoidMethod(jGUI, jSetSpectroData, jSpectro,
				jint(spectroData->getSizeF()),
				jdouble(spectroData->getScaleT()),
				jdouble(spectroData->getLowThreshold()),
				jdouble(spectroData->getHighThreshold()),
				jdouble(spectroData->getOverlap()));
		jAssert(TRUE, "Calling Java method Gui.setSpectroData()");
	}
} // gui :: updateAudioDisplay()

void gui :: showWarningDialog(const char* message) {
	jstring jmessage = (jstring) env->NewStringUTF(message);
	if (!jAssert(jmessage != NULL, "Can't create string jmessage"))
		return;
	env->CallVoidMethod(jGUI, jShowMessage, jmessage);
	jAssert(TRUE, "Calling Java method Gui.showMessage()");
} // gui :: showWarningDialog()

void gui :: processRequests() {
	eventList* midiSalience = NULL;
	while (1) try {		// event loop (reading events from Java queue)
		jstring jrequest = (jstring) env->CallObjectMethod(jGUI, jGetRequest);
		if (jrequest != NULL) {
			const char *s = env->GetStringUTFChars(jrequest, NULL);
			if (strcmp(s, "_quit_") == 0) {
				playq->interrupted = true;
				mySleep(100000);
				exit(0);
			}
			if (parameters->debug("gui", "basic"))
				cerr << "PROCESSING: " << s << endl;
			char* fn = strchr(s, ':');
			if (fn == NULL) {
				cerr << "Ignoring illegal GUI request\n" << s << endl;
				env->ReleaseStringUTFChars(jrequest, s);
				continue;
			}
			*fn++ = 0;
			char* args = strchr(fn, ':');
			if (args == NULL) {
				cerr << "Ignoring illegal GUI request\n" << s <<":" <<fn <<endl;
				env->ReleaseStringUTFChars(jrequest, s);
				continue;
			}
			*args++ = 0;
//			parameters->clear();					// Can't clear coz -P option
//			parameters->readDefaults();				// No clear -> no reread
			parameters->add("selectedStart=0");		// Clear previous selection
			parameters->add("selectedLength=-1");	// ... hope that's enough
			parameters->readString(args);			// Process args from GUI
			mode = parameters->getInt("blindMode", SHOW_SPECTRO);
			bool trackingDisabled = (strcmp(
					parameters->getString("disable", "off"), "on") == 0);
			setMode(0, mode);
			double start = parameters->getDouble("selectedStart", 0.0);
			double len = parameters->getDouble("selectedLength", -1.0);
			if (!strcmp(s, "New")) {	// perform beat tracking on new data
				if (audioData) { delete audioData; audioData = NULL; }
				if (audioPlayData) { delete audioPlayData; audioPlayData=NULL; }
				if (spectroData) { delete spectroData; spectroData = NULL; }
				if (audioEnv) { delete audioEnv; audioEnv = NULL; }
				if (midiData) { delete midiData; midiData = NULL; }
				if (midiSalience) { delete midiSalience; midiSalience = NULL; }
				if (beatData) { delete beatData; beatData = NULL; }
				if (undoData) { delete undoData; undoData = NULL; }
				fn = parameters->getString("fixedInputFile", fn);
				if (endsWith(fn,".mid",true) || endsWith(fn,".tmf",true) ||
							(endsWith(fn, ".match", true))) {
					setMode(SHOW_MIDI, SHOW_AUDIO | SHOW_SPECTRO | mode);
					midiData = new eventList(fn);
					midiData->deleteMidiClickTrack();
					char* af = parameters->getString("audioFile");
					if (af != NULL) {
						audioData = new sample(af);
						midiData->align(parameters->getDouble("firstNote",-1));
					}
					updateMidiDisplay();
					midiSalience = midiData->calculateMidiSalience();
					beatData = trackingDisabled? new eventList():
												 midiSalience->beatTrack();
					assertWarning(trackingDisabled ||
							(beatData->next->ev!=NULL), "Beat tracking failed");
					updateBeatDisplay();
				} else {	// assume it is audio data
					audioData = new sample(fn);
					if (audioData->data == NULL) {
						delete audioData;
						audioData = NULL;
					} else {
						midiData = audioData->getOnsets(&audioEnv);
                        if ((mode & SHOW_SPECTRO) == 0) {
							spectroData = new spectrogram(audioData);
							setMode(SHOW_AUDIO | SHOW_SPECTRO, SHOW_MIDI |mode);
						} else
							setMode(SHOW_AUDIO, SHOW_SPECTRO | SHOW_MIDI |mode);
						updateAudioDisplay();
						beatData = trackingDisabled? new eventList():
													 midiData->beatTrack();
						assertWarning(trackingDisabled ||
							(beatData->next->ev!=NULL), "Beat tracking failed");
						updateBeatDisplay();
					}
				}
			} else if (!strcmp(s, "Load Beats")) {	// load beat track data
				updateBeatData();
				rememberBeatData();
				if (beatData) { delete beatData; beatData = NULL; }
				beatData = new eventList(fn);
				beatData->deleteMidiNonClickTrack();
				updateBeatDisplay();
			} else if (!strcmp(s, "Clear")) {		// clear beat data
				updateBeatData();
				rememberBeatData();
				if (beatData != NULL) {
					beatData->clear(start, len);
					updateBeatDisplay();
				}
			} else if (!strcmp(s, "Cut File")) {
				if (midiData != NULL) {
					midiData->deleteMidiClickTrack();
					midiData->writeTMF(fn, start, len);
				}
			} else if (!strncmp(s, "Save Beats", 10)) {	// save beat track data
				updateBeatData();
				beatData->writeTMF(fn); 	// , start, len); // ??
			} else if (!strcmp(s, "Play")) {		// play audio or midi
				if (parameters->getString("deafMode") == NULL) {
					updateBeatData();
					if (audioPlayData != NULL)
						audioPlayData->play(start, len);
					else if (midiData != NULL)
						midiData->play(start, len);
				}
			} else if (!strcmp(s, "Re-track")) {	// re-do beat tracking
				if (trackingDisabled)
					assertWarning(FALSE, "Beat tracking has been disabled");
				else {
					updateBeatData();
					rememberBeatData();
					if (midiData != NULL) {
						eventList* tmp;
						if (midiSalience != NULL)
							tmp = midiSalience->beatTrack(beatData, start);
						else
							tmp = midiData->beatTrack(beatData, start);
						if (assertWarning(tmp->next->ev != NULL,
											"Beat tracking failed")) {
							if (beatData != NULL) delete beatData;
							beatData = tmp;
						} else if (beatData != NULL)
							beatData->clear(start);
						updateBeatDisplay();
					}
				}
			} else if (!strcmp(s, "Stop")) {		// stop playing data
				playq->interrupted = true;
			} else if (!strcmp(s, "Save All")) {	// write audio/midi to file
				updateBeatData();
				if (audioPlayData != NULL)
					audioPlayData->writeFile(fn, start, len);
				else if (midiData != NULL)
					midiData->writeTMF(fn, start, len);
			} else if (!strcmp(s, "Undo")) {		// i didn't mean it, really
				updateBeatData();
				restoreBeatData();
				updateBeatDisplay();
			} else
				cerr << "Unrecognised command: " << s << endl;
			env->ReleaseStringUTFChars(jrequest, s);
		} else
			mySleep(100000);
	} catch (Exception e) {
		assertWarning(false, e.message());
	}
} // gui :: processRequests()

