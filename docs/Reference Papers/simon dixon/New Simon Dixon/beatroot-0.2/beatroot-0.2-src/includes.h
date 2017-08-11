//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: includes.h
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

// Mainly platform specific system includes - currently Linux

#ifndef _BT_INCLUDES_H_
#define _BT_INCLUDES_H_


// for istream, ostream
#include <iostream>

// for setw, setprecision
#include <iomanip>

// for ifstream, ofstream
#include <fstream>

// for ostrstream
#include <strstream>

// for ioctl(), write(), close()
#include <unistd.h>
#include <sys/ioctl.h>

// for sound on Linux
#include <linux/soundcard.h>

// for O_RDONLY, O_RDWR
#include <fcntl.h>

// for I_FLUSH, FLUSHR, FLUSHRW
// #include <stropts.h>

// for atoi()
#include <cstdlib>

// for sin(), rint(), M_PI
#include <cmath>

// for [ fs]printf()
// #include <cstdio>

// for memset(), strcmp(), strcpy(), etc
#include <cstring>

// for string
#include <string>

// for isdigit(), toupper()
#include <cctype>

// for sig_action() SIGINT, SIGALARM, etc
#include <csignal>

// for setitimer()
#include <sys/time.h>

// for getpwuid(), getuid()
#include <pwd.h>

#define TRUE 1
#define FALSE 0

// forward definitions for global objects
class parameterList;
class gui;
class playQ;

// the 3 global objects
extern parameterList* parameters;
extern gui* jgui;
extern playQ* playq;

#endif
