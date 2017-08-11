//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: local.h
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

typedef short int16;
typedef int int32;
typedef int16 audioSample;
#define minAudioSample -32768
#define maxAudioSample 32767
#define AUDIO_FORMAT AFMT_S16_LE
#define audioLimit(x) (x<minAudioSample? minAudioSample: \
                        (x>maxAudioSample? maxAudioSample: x));
