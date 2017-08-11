//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: param.h
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

#ifndef NULL
#define NULL 0
#endif

#include <iostream>

class parameterList {
	char **data;
	int count;
	int size;
	bool showDefaults;
public:
	parameterList(int size = 250);

	void add(char* s);
	void remove(char* s);
	void clear();
	int find(char* s);
	double getDouble(char* s, double defaultValue = 0.0);
	int getInt(char* s, int defaultValue = 0);
	char* getString(char* s, char* defaultValue = NULL);
	void write(ostream& out = cout);
	void read(istream& in = cin);
	void readDefaults();
	void readString(char* str);
	bool debug(char* prefix, char* suffix);
}; // class parameterList
