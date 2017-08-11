//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: param.cpp
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

// Implementation of CLASS PARAMETERLIST

#include "includes.h"
#include "param.h"
#include "util.h"

parameterList :: parameterList(int size) {
	data = new char*[size];
	count = 0;
	this->size = size;
	showDefaults=false;
} // parameterList constructor

int parameterList :: find(char* s) {
	for (int i=0; i<count; i++)
		if (strncmp(s,data[i],strlen(s)) == 0)
			return i;
	return -1;
} // find()

void parameterList :: add(char* s) {
	if (strcmp(s, "debug:main=showDefaults") == 0)
		showDefaults = true;
	if (!assertWarning((s != NULL) && (*s != 0), "Empty parameter"))
		return;
	if (!assertWarning(strchr(s, '=') != NULL, "Illegal parameter (ignored)"))
		return;
	s = charArrayCopy(s);
	char *p = strchr(s, '=');
	(*p) = '\0';
	int i = find(s);
	(*p) = '=';
	if (i >= 0)		// replace previous parameter value
		data[i] = s;
	else if (assertWarning(count < size, "Overflow: can't set parameter"))
		data[count++] = s;
} // add()

void parameterList :: remove(char* s) {
	int i = find(s);
	if (i >= 0) {
		delete data[i];
		for (; i < count-1; i++)
			data[i] = data[i+1];
		count--;
	}
} // remove()

void parameterList :: clear() {
	for (int i=0; i<count; i++)
		delete[] data[i];
	count = 0;
} // clear()

double parameterList :: getDouble(char* s, double defaultValue) {
	int i = find(s);
	if (i >= 0)
		return atof(strchr(data[i],'=')+1);
	if (showDefaults)
		cout << "Using default parameter setting: " << s << " = "
			 << setprecision(3) << defaultValue << endl;
	return defaultValue;
} // getDouble()

int parameterList :: getInt(char* s, int defaultValue) {
	int i = find(s);
	if (i >= 0)
		return atoi(strchr(data[i],'=')+1);
	if (showDefaults)
		cout << "Using default parameter setting: " << s << " = "
			 << defaultValue << endl;
	return defaultValue;
} // getInt()

char* parameterList :: getString(char* s, char* defaultValue) {
	int i = find(s);
	if (i >= 0)
		return strchr(data[i],'=')+1;
	if (showDefaults)
		cout << "Using default parameter setting: " << s << " = "
			 << defaultValue << endl;
	return defaultValue;
} // getString()

void parameterList :: write(ostream& out) {
	for (int i=0; i<count; i++)
		out << data[i] << "\n";
} // print()

void parameterList :: read(istream& in) {
	char buf[256];
	while (in.getline(buf, 256)) {
		if ((strlen(buf) > 0) && !in.fail() && isalnum(buf[0]))
			add(buf);
	}
} // read()

// Read default parameter settings from .beatrootrc file in user's home dir
void parameterList :: readDefaults() {
	struct passwd *pwEntry = getpwuid(getuid());
	if (pwEntry != NULL) {
		string s(pwEntry->pw_dir);
		s += "/.beatrootrc";
		ifstream st(s.c_str());
		this->read(st);
	}
} // readDefaults()

void parameterList :: readString(char* s) {
	char* ptr;
	do {
		ptr = strchr(s, ' ');
		if (ptr != NULL)
			*ptr = 0;
		if (*s != 0)
			add(s);
		s = ptr + 1;
	} while (ptr != NULL);
} // readString()

bool parameterList :: debug(char* prefix, char* suffix) {
	char buff[100];
	strcpy(buff, "debug:");
	strcat(buff, prefix);
	char* d = getString(buff);
	return ((d != NULL) && ((*d == '*') || (strstr(d, suffix) != NULL)));
} // debug()

// END OF CLASS PARAMETER_LIST
