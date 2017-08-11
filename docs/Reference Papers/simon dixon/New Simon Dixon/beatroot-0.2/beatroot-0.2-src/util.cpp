//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: util.cpp
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

#include "gui.h"
#include "includes.h"
#include "local.h"
#include "param.h"
#include "util.h"

void errorCheck(int err, const char* mesg) {
	if (err) {
		perror("perror says: ");
		cerr << "Error: " << mesg << endl;
		if (parameters->debug("main", "abort"))
			abort();
		else
			exit(1);
	}
} // errorCheck()

void asserts(int flag, const char* mesg) {
	errorCheck(!flag, mesg);
} // asserts()

int assertWarning(int flag, const char* mesg) {
	if (!flag) {
		if (jgui != NULL)
			jgui->showWarningDialog(mesg);
		cerr << "Warning: " << mesg << endl;
	}
	return flag;
} // assertWarning()

// Returns value, clipped to lie within the range [min, max]
double threshold(double value, double min, double max) {
    return value < min ? min : (value > max ? max : value);
} // threshold()

// Swaps elements i and j of a[]
void swap(int* a, int i, int j) {
	int temp = a[i];
	a[i] = a[j];
	a[j] = temp;
} // swap()

// Swaps elements i and j of a[]
void swap(double* a, int i, int j) {
	double temp = a[i];
	a[i] = a[j];
	a[j] = temp;
} // swap()

int max(int* a, int sz, int start = 0, int stop = -1) {
	if ((stop < 0) || (stop > sz))
		stop = sz;
	if (start < 0)
		start = 0;
	int ans = a[start];
	for ( ; start < stop; start++)
		if (ans < a[start])
			ans = a[start];
	return ans;
} // max()

double max(double* a, int sz, int start = 0, int stop = -1) {
	if ((stop < 0) || (stop > sz))
		stop = sz;
	if (start < 0)
		start = 0;
	double ans = a[start];
	for ( ; start < stop; start++)
		if (ans < a[start])
			ans = a[start];
	return ans;
} // max()

// Sorts array a[size] using quickSort algorithm (average performance O(nlogn))
void quickSort(double* a, int size) {
	int lo = 0;
	int hi = size-1;
	double pivot;
	if (hi > 0) {
		if (a[hi] < a[0])
			swap(a, hi, 0);
		pivot = a[0];
		while (TRUE) {
			do {
				lo++;
			} while (a[lo] < pivot);
			do {
				hi--;
			} while (a[hi] > pivot);
			if (lo > hi)
				break;
			swap(a, lo, hi);
		} 
		swap(a, 0, hi);
		quickSort(a, hi);
		quickSort(a+hi+1, size-hi-1);
	}
} // quickSort()

// Finds the interval of given width with the largest number of values in
//  a[size], and returns the mean of the values in this interval.
// Assumes a[] is sorted.
double findMode(const double* a, int size, double width) {
	int length, bestLength, start, bestStart, i;
	bestLength = length = i = 0;
	for (start = 0; start < size; start++) {
		for ( ; (i < size) && (a[i] - a[start] < width); i++)
			length++;
		if (length > bestLength) {
			bestLength = length;
			bestStart = start;
		}
		length--;
	}
	if (parameters->debug("event", "mode")) {
		cout.setf(ios::fixed, ios::scientific);
		for (i=0; i < size; i++)
			cout << setprecision(3) << setw(7) << a[i]
				 << (i%10 == 9 ? '\n': ' ');
		cout << "\nBest at " << bestStart << " with length " <<bestLength<<endl;
	}
	double sum = 0;
	for (i = 0; i < bestLength; i++)
		sum += a[bestStart + i];
	return sum / double(bestLength);
} // findMode()

// Substitutes all occurrences of oldChar in s with newChar
void gsub(char* s, char oldChar, char newChar) {
	while (s = strchr(s, oldChar))
		*s = newChar;
} // gsub()

// Returns a copy of s
char* charArrayCopy(const char *s) {
	char* n = new char[strlen(s)+1];
	strcpy(n, s);
	return n;
} // charArrayCopy()

// Checks if string s starts with string prefix
int startsWith(const char* s, const char* prefix, bool ignoreCase) {
	return  (ignoreCase ? !strncasecmp(prefix, s, strlen(prefix)) :
						  !strncmp(prefix, s, strlen(prefix)));
} // startsWith()

// Checks if string s ends with string suffix
int endsWith(const char* s, const char* suffix, bool ignoreCase) {
	int prefixLength = strlen(s) - strlen(suffix);
	return (prefixLength >= 0) &&
			(ignoreCase ? !strcasecmp(suffix, s+prefixLength) :
						  !strcmp(suffix, s+prefixLength));
} // endsWith()

// Returns true if input (from f) matches s;
// otherwise returns false without changing f.
bool matchString(istream& f, const char* s, bool rewind = true) {
	const char* p = s;
	while (*p && f) {
		if (f.peek() != *p)
			break;
		p++;
		f.get();
	}
	if (*p == 0)
		return true;
	while (rewind && (p != s))
		f.putback(*--p);
	return false;
} // matchString()

// Reads chars from f until c is found;
// reports an error if eoln or eof is found before c.
void skip(ifstream& f, char c) throw (Exception) {
	while (f) {
		char tmp = f.get();
		if (tmp == c)
			return;
		if (tmp == '\n')
			throw Exception("Parse error: Unexpected end of line");
	}
	throw Exception("Parse error: Unexpected end of file");
} // skip()

void skipLine(istream& f) {
	for (int c = f.get(); c != '\n'; c = f.get())
		if (c == '\\')
			c = f.get();    // skip extra character
} // skipLine

void binaryString(char* s, int b, int count) {
	for (int i = count-1; i >= 0; i--) {
		*s++ = (b & (1 << i)) ? '1' : '0';
		*s++ = ' ';
	}
	*s = 0;	// string terminator
} // binaryString()

// Checks if filename matches extension (which may be overridden by format)
int fileNameMatch(const char* fileName, const char* format, const char* extn) {
	if (format != NULL)
		return !strcmp(format, extn);
	return endsWith(fileName, extn);
} // fileNameMatch()

// Creates a new file name in dest consisting of fileName with the path
//  and extension replaced by the path and extension variables (if not NULL)
void makeFileName(const char* fileName, const char* path,
				  const char* extension, char* dest) {
	*dest = 0;
	asserts(strlen(fileName) < 200, "Filename too long");
	if (path != NULL)
		strcpy(dest, path);
	if ((path != NULL) && strrchr(fileName, '/'))
		strcat(dest, strrchr(fileName, '/') + 1);	// remove old path
	else
		strcat(dest, fileName);
	if ((extension != NULL) && strrchr(dest, '.'))
		*(strrchr(dest, '.')) = 0;					// remove old extension
	if (extension != NULL)
		strcat(dest, extension);					// add new extension
} // makeFileName()

// Returns frequency in Hertz for a given MIDI note number; A4 = 440.0Hz = #69
double midiFrequency(int noteNumber) {
	return parameters->getDouble("tune", 440.0) *
				pow(2.0, double(noteNumber - 69) / 12.0);
}

// Returns nearest MIDI note no. for a frequency given in Hz; A4 = 440.0Hz = #69
int midiNumber(double frequency) {
	return 69 + int( rint( log( frequency / parameters->getDouble("tune",440.0))
						/ log(2.0) * 12.0));
}

// Prints an array as Matlab code to recreate the array
void printToMatlab(const double* theArray, int size, const char* arrayName) {
	cout << arrayName << " = [" << endl;
	for (int i = 0; i < size; i++)
		cout << setw(16) << setprecision(8) << theArray[i] << endl;
	cout << "];" << endl;
}

// Dummy signal handler used by mySleep
void alarmHandler(int signum) {
}

// Sleeps for a specified number of microseconds
// The system version of sleep() is too coarse as it counts in seconds.
// Warning: it is implemented with SIGALRM; i.e. resets the alarm timer 
void mySleep(int microseconds) {
	static struct timeval t = { 0L, 0L }; // wait 0.1 sec (+10ms)
	static struct timeval tz = { 0L, 0L };
	t.tv_usec = microseconds;
	struct itimerval it = {tz, t};
	setitimer(ITIMER_REAL, &it, 0);
	struct sigaction act = { alarmHandler, SA_NOMASK, SA_RESETHAND, 0 };
	sigaction(SIGALRM, &act, 0);
	pause();
} // mySleep()

// For cross-platform reading and writing of data e.g. audio
int32 sRead32BE(const unsigned char* s) {
    return  ((int32)s[0] << 24) |
			((int32)s[1] << 16 & 0xFF0000) |
			((int32)s[2] << 8 & 0xFF00) |
			((int32)s[3] & 0xFF);
}

void sWrite32BE(int32 i, unsigned char* buff) {
	buff[0] = i >> 24 & 0xFF;
	buff[1] = i >> 16 & 0xFF;
	buff[2] = i >> 8 & 0xFF;
	buff[3] = i & 0xFF;
}

int16 sRead16BE(const unsigned char* s) {
    return (int16) (((int)s[0] << 8 & 0xFF00) |
					((int)s[1] & 0xFF));
}

int16 read16BE(istream& f) {
	unsigned char buff[2];
	f.read(buff, 2);
	if (!assertWarning(f.gcount() == 2, "read16BE: read error"))
		return 0;
	return (int16)(((int)buff[0] << 8) + ((int)buff[1] & 0xFF));
}

void write24BE(int i, ostream& f) {
	unsigned char buff[3];
	buff[0] = i >> 16 & 0xFF;
	buff[1] = i >> 8 & 0xFF;
	buff[2] = i & 0xFF; 
	f.write(buff,3);
}

void write16BE(int16 i, ostream& f) {
	unsigned char buff[2];
	buff[0] = i >> 8 & 0xFF;
	buff[1] = i & 0xFF; 
	f.write(buff,2);
}

void write32BE(int32 i, ostream& f) {
	unsigned char buff[4];
	sWrite32BE(i, buff);
	f.write(buff,4);
}

int32 sRead32LE(const unsigned char* s) {
    return  ((int32)s[3] << 24) |
			((int32)s[2] << 16 & 0xFF0000) |
			((int32)s[1] << 8 & 0xFF00) |
			((int32)s[0] & 0xFF);
}

void sWrite32LE(int32 i, unsigned char* buff) {
	buff[3] = i >> 24 & 0xFF;
	buff[2] = i >> 16 & 0xFF;
	buff[1] = i >> 8 & 0xFF;
	buff[0] = i & 0xFF;
}

int16 sRead16LE(const unsigned char* s) {
    return (int16) (((int)s[1] << 8 & 0xFF00) |
					((int)s[0] & 0xFF));
}

int16 read16LE(istream& f) {
	unsigned char buff[2];
	f.read(buff, 2);
	if (!assertWarning(f.gcount() == 2, "read16BE: read error"))
		return 0;
	return sRead16LE(buff);
}

void write8(int8 i, ostream& f) {
	f.write(&i, 1);
}

void write16LE(int16 i, ostream& f) {
	unsigned char buff[2];
	buff[1] = i >> 8 & 0xFF;
	buff[0] = i & 0xFF; 
	f.write(buff,2);
}

void write32LE(int32 i, ostream& f) {
	unsigned char buff[4];
	sWrite32LE(i, buff);
	f.write(buff,4);
}

inline int hextoi(char c) {
	return isdigit(c)? c - '0': (toupper(c) - 'A' + 10);
}
inline char itohex(int c) {
	return (c > 9) ? (c + 'A' - 10) : (c + '0');
}

// Writes a string representing a hexadecimal number as a sequence of bytes
void writeHexString(const char* data, ostream& f) {
	int len = strlen(data);
	assertWarning((len & 1) == 0, "Illegal length in writeHex()");
	for (int i = 0; i < len-1; i += 2) {
		unsigned char value = (hextoi(data[i]) << 4) | hextoi(data[i+1]);
		f.write(&value, 1);
	}
}

// Reads a sequence of bytes and returns an ASCII representation
char* readHexString(istream& f, int length) {
	char* out = new char[length*2+1];
	unsigned char c;
	for (int i=0; i<length; i++) {
		f >> c;
		out[2*i] = itohex(c >> 4);
		out[2*i+1] = itohex(c & 0x0F);
	}
	out[2*length] = 0;
	return out;
}
