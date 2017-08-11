# Makefile for beat tracking / beat mixing program
# Author: Simon Dixon <simon@ai.univie.ac.at>
# Modification date: 12/12/2001

PROJ = run
INSTALLD = /raid/music/bin
CC = /usr/bin/gcc
JAVAD = /usr/lib/jdk1.3
JAVAC = $(JAVAD)/bin/javac
CFLAGS = -g \
	-I$(JAVAD)/include \
	-I$(JAVAD)/include/linux \
	-L$(JAVAD)/jre/lib/i386 \
	-L$(JAVAD)/jre/lib/i386/native_threads
#	-v
#	-Wall
LINKFLAGS = \
	-Xlinker -rpath \
	-Xlinker $(JAVAD)/jre/lib/i386 \
	-Xlinker -rpath \
	-Xlinker $(JAVAD)/jre/lib/i386/classic \
	-Xlinker -rpath \
	-Xlinker $(JAVAD)/jre/lib/i386/native_threads
LIBS = -lstdc++ -lm -lpthread -ljava -lhpi
OBJS = \
		agent.o \
		event.o \
		eventBeat.o \
		eventInduction.o \
		eventMidi.o \
		fft.o \
		gui.o \
		main.o \
		onset.o \
		param.o \
		playq.o \
		realTimeInduction.o \
		realTimeSystem.o \
		realTimeTracking.o \
		sample.o \
		sampleAdv.o \
		sampleBeat.o \
		sampleIO.o \
		sfheader.o \
		song.o \
		spectrum.o \
		tmf.o \
		tree.o \
		util.o
#		learn.o \

CLASSES = \
	BeatTrackDisplay.class \
	ControlPanel.class \
	Gui.class \
	JobQ.class \
	PanelScroller.class \
	ZoomListener.class

all: $(PROJ) $(CLASSES)

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) $(LINKFLAGS) -o $(PROJ) $(OBJS) $(LIBS)

clean:
	/bin/rm $(OBJS) # $(PROJ) *.class

install: all
	/bin/cp *.class $(INSTALLD)/classes/. ; \
	/bin/cp $(PROJ) $(INSTALLD)/beatroot ; \
	/bin/chmod a+r $(INSTALLD)/classes/*.class ; \
	/bin/chmod a+rx $(INSTALLD)/beatroot

agent.o: agent.cpp agent.h event.h includes.h param.h realTime.h tmf.h \
 util.h local.h
	$(CC) $(CFLAGS) -c -o agent.o agent.cpp

event.o: event.cpp event.h includes.h param.h playq.h local.h tmf.h util.h
	$(CC) $(CFLAGS) -c -o event.o event.cpp

eventBeat.o: eventBeat.cpp includes.h agent.h event.h param.h tmf.h \
 util.h local.h
	$(CC) $(CFLAGS) -c -o eventBeat.o eventBeat.cpp

eventInduction.o: eventInduction.cpp includes.h agent.h event.h tmf.h \
 param.h util.h local.h
	$(CC) $(CFLAGS) -c -o eventInduction.o eventInduction.cpp

eventMidi.o: eventMidi.cpp includes.h event.h param.h tmf.h \
 tree.h util.h local.h
	$(CC) $(CFLAGS) -c -o eventMidi.o eventMidi.cpp

fft.o: fft.cpp includes.h fft.h sample.h local.h sfheader.h util.h
	$(CC) $(CFLAGS) -c -o fft.o fft.cpp

gui.o: gui.cpp event.h gui.h local.h param.h playq.h sample.h sfheader.h \
 includes.h tmf.h util.h
	$(CC) $(CFLAGS) -c -o gui.o gui.cpp

learn.o: learn.cpp event.h param.h util.h includes.h tmf.h
	$(CC) $(CFLAGS) -c -o learn.o learn.cpp

main.o: main.cpp event.h gui.h param.h playq.h local.h realTime.h \
 sample.h sfheader.h includes.h spectrum.h tmf.h util.h
	$(CC) $(CFLAGS) -c -o main.o main.cpp

onset.o: onset.cpp event.h includes.h param.h sample.h util.h
	$(CC) $(CFLAGS) -c -o onset.o onset.cpp

param.o: param.cpp includes.h param.h util.h local.h
	$(CC) $(CFLAGS) -c -o param.o param.cpp

playq.o: playq.cpp includes.h param.h playq.h local.h sample.h sfheader.h util.h
	$(CC) $(CFLAGS) -c -o playq.o playq.cpp

realTimeInduction.o: realTimeInduction.cpp includes.h event.h param.h \
 realTime.h tmf.h util.h local.h
	$(CC) $(CFLAGS) -c -o realTimeInduction.o realTimeInduction.cpp

realTimeSystem.o: realTimeSystem.cpp agent.h event.h includes.h \
 param.h realTime.h tmf.h
	$(CC) $(CFLAGS) -c -o realTimeSystem.o realTimeSystem.cpp

realTimeTracking.o: realTimeTracking.cpp agent.h event.h includes.h \
 param.h realTime.h tmf.h
	$(CC) $(CFLAGS) -c -o realTimeTracking.o realTimeTracking.cpp

sample.o: sample.cpp includes.h param.h playq.h local.h sample.h \
 sfheader.h util.h
	$(CC) $(CFLAGS) -c -o sample.o sample.cpp

sampleAdv.o: sampleAdv.cpp sample.h local.h sfheader.h includes.h util.h
	$(CC) $(CFLAGS) -c -o sampleAdv.o sampleAdv.cpp

sampleBeat.o: sampleBeat.cpp event.h param.h sample.h local.h \
 sfheader.h includes.h util.h tmf.h
	$(CC) $(CFLAGS) -c -o sampleBeat.o sampleBeat.cpp

sampleIO.o: sampleIO.cpp param.h playq.h local.h sample.h sfheader.h \
 includes.h util.h
	$(CC) $(CFLAGS) -c -o sampleIO.o sampleIO.cpp

sfheader.o: sfheader.cpp local.h sfheader.h includes.h util.h
	$(CC) $(CFLAGS) -c -o sfheader.o sfheader.cpp

song.o: song.cpp sample.h local.h param.h sfheader.h includes.h song.h util.h
	$(CC) $(CFLAGS) -c -o song.o song.cpp

spectrum.o: spectrum.cpp includes.h event.h fft.h param.h sample.h \
 local.h sfheader.h spectrum.h util.h tmf.h
	$(CC) $(CFLAGS) -c -o spectrum.o spectrum.cpp

tmf.o: tmf.cpp event.h includes.h param.h tmf.h util.h
	$(CC) $(CFLAGS) -c -o tmf.o tmf.cpp

tree.o: tree.cpp event.h includes.h tree.h util.h local.h tmf.h
	$(CC) $(CFLAGS) -c -o tree.o tree.cpp

util.o: util.cpp local.h param.h util.h includes.h gui.h
	$(CC) $(CFLAGS) -c -o util.o util.cpp

BeatTrackDisplay.class: BeatTrackDisplay.java
	rm BeatTrackDisplay*.class; $(JAVAC) BeatTrackDisplay.java

ControlPanel.class: ControlPanel.java
	rm ControlPanel*.class; $(JAVAC) ControlPanel.java

Gui.class: Gui.java
	rm Gui*.class; $(JAVAC) Gui.java

JobQ.class: JobQ.java
	rm JobQ*.class; $(JAVAC) JobQ.java

PanelScroller.class: PanelScroller.java
	rm PanelScroller*.class; $(JAVAC) PanelScroller.java

ZoomListener.class: ZoomListener.java
	rm ZoomListener*.class; $(JAVAC) ZoomListener.java
