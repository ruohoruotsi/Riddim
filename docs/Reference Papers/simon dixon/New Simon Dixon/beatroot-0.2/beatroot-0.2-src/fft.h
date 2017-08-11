//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: fft.h
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

#define RECT 0
#define HAMMING 1
#define BH3 2
#define BH4 3
#define BH3MIN 4
#define BH4MIN 5
#define GAUSS 6

typedef struct {
	double r, i;
} complex;

void fft(double* data, int size);
complex dftComponent(double* data, int size, double freq);
double* windowChoice(int choice, int size, int support);
void applyWindow(double* data, double* wdata, int size);
