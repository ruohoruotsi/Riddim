//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: util.h
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
#include "local.h"

void errorCheck(int err, const char* mesg);
void asserts(int flag, const char* mesg = "Assertion failed");
int assertWarning(int flag, const char* mesg = "Assertion failed");

class Exception {
	private:
		const char* mesg;
	public:
		Exception(const char* errMesg) : mesg(errMesg) {}
		void print() { cerr << "Exception: " << mesg << endl; }
		const char* message() { return mesg; }
};

void mySleep(int microseconds);

double threshold(double value, double min, double max);
void swap(int* a, int i, int j);
void swap(double* a, int i, int j);
int max(int* a, int sz, int start, int stop);
double max(double* a, int sz, int start, int stop);
void quickSort(double* a, int size);
double findMode(const double* a, int size, double width);

void gsub(char* s, char toReplace, char replaceWith);
char* charArrayCopy(const char *s);
int startsWith(const char* string, const char* prefix, bool ignoreCase = false);
int endsWith(const char* string, const char* suffix, bool ignoreCase = false);
bool matchString(istream& f, const char* s, bool rewind = true);
void skip(ifstream& f, char c) throw (Exception);
void skipLine(istream& f);
void binaryString(char* s, int b, int count);

double midiFrequency(int midiNumber);
int midiNumber(double frequency);

int fileNameMatch(const char* fileName, const char* format, const char* extn);
void makeFileName(const char* fileName, const char* prefix,
				  const char* suffix, char* dest);

void printToMatlab(const double* a, int size, const char* name);

typedef unsigned char int8;
typedef short int16;
typedef int int32;

int32 sRead32BE(const unsigned char* s);
int32 sRead32LE(const unsigned char* s);
int16 sRead16BE(const unsigned char* s);
int16 sRead16LE(const unsigned char* s);
void sWrite32BE(int32 i, unsigned char* buff);
void sWrite32LE(int32 i, unsigned char* buff);

int16 read16BE(istream& f);
int16 read16LE(istream& f);
void write8(int8 i, ostream& f);
void write16BE(int16 i, ostream& f);
void write16LE(int16 i, ostream& f);
void write24BE(int i, ostream& f);
void write32BE(int32 i, ostream& f);
void write32LE(int32 i, ostream& f);

inline int hextoi(char c);
inline char itohex(int i);
void writeHexString(const char* data, ostream& f);
const char* readHexString(istream& f);
