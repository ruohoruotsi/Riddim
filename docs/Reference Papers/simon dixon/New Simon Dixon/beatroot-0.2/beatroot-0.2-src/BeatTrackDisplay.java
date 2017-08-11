//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: BeatTrackDisplay.java
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class BeatTrackDisplay extends JPanel {

	Gui gui;					// handle to the Gui (parent) object
	int xSize, ySize;			// display area (pixels)
	int yTop;					// top of data area
	int yMid;					// division between spectro and audio area
	int yBottom;				// bottom of data area (position of x-axis)
	int selectedBeat;			// index of selected beat
	double startSelection;		// beginning of selected data region
	double endSelection;		// end of selected data region (may be < start)
	double[] onsets;			// onset times of events (seconds)
	double[] offsets;			// offset times of events (seconds)
	int[] pitches;				// MIDI pitches of events
	double[] env;				// audio env times corresponding to magnitudes
	int[] magnitudes;			// magnitude of audio env
	double[] beats;				// positions of beats (seconds)
    double tInc;				// time interval between columns of spectro data
    double[] spectro;			// spectrogram data
    int pitchCount;				// number of rows per column of spectro data
    Color[] colour;				// colours for spectrogram
	double hiThreshold, loThreshold, overlap;	// parameters for spectrogram
	double minimumTime;			// time of first onset or beat (seconds)
	double maximumTime;			// time of last onset or beat (seconds)
	double visibleTimeStart;	// starting time (seconds)
	double visibleTimeLength;	// visible duration (seconds)
	int midiMin;				// smallest displayed midi number
	int midiMax;				// largest displayed midi number
	static final int NO_BEAT_SELECTED = -1;
	static final int REGION_SELECTED = -2;
	Font font;					// font used for all displayed text
	FontMetrics metrics;		// for calculating size of printed strings
	public static final int SHOW_BEATS = 1;
	public static final int SHOW_IBI = 2;
	public static final int SHOW_DATA = 4;
	public static final int SHOW_ONSETS = 8;
	public static final int SHOW_AUDIO = 1024;		// DO NOT CHANGE - see gui.h
	public static final int SHOW_SPECTRO = 2048;	// DO NOT CHANGE - see gui.h
	public static final int SHOW_MIDI = 4096;		// DO NOT CHANGE - see gui.h
	public static final int DEFAULT_MODE = SHOW_IBI | SHOW_ONSETS |
										   SHOW_DATA | SHOW_BEATS;
	int mode;

	public BeatTrackDisplay(Gui g) {
		gui = g;
		mode = DEFAULT_MODE;
		midiMin = 21;
		midiMax = 108;
		onsets = new double[0];
		offsets = new double[0];
		pitches = new int[0];
		beats = new double[0];
		env = new double[0];
		magnitudes = new int[0];
        makeColours();
		setBorder(BorderFactory.createLineBorder(Color.black));
		font = new Font("Helvetica", Font.PLAIN, 12);
		g.setFont(font);
		metrics = getFontMetrics(font);
		BTDMouseListener ml = new BTDMouseListener(this);
		addMouseListener(ml);
		addMouseMotionListener(ml);
		init(true);
	} // BeatTrackDisplay constructor

	void show(String s, double[] arr, int i) {
		System.err.println(s + "(length = " + arr.length + ")");
		for (int count = 10; i < arr.length; i++) {
			System.err.print(arr[i] + " ");
			if (count-- == 0) break;
		}
		System.err.println();
	}
		
	void show(String s, int[] arr, int i) {
		System.err.println(s + "(length = " + arr.length + ")");
		for (int count = 10; i < arr.length; i++) {
			System.err.print(arr[i] + " ");
			if (count-- == 0) break;
		}
		System.err.println();
	}
	
    void makeColours() {
        colour = new Color[256];
        for (int i = 0; i < 256; i++)
            colour[i] = new Color(i,i,i);
    } // makeColours()

    Color getColour(double c) {
        int index = (int)(255 - (c - loThreshold) *
							255.0 / (hiThreshold - loThreshold));
        return colour[index < 0? 0: (index > 255? 255: index)];
    } // getColour()

	synchronized public void init(boolean resetSelection) {
		// System.err.println("Init Called with reset=" + resetSelection);
		// show("onsets", onsets, 0);
		// show("offsets", offsets, 0);
		// show("beats", beats, 0);
		// show("pitches", pitches, 0);
		// show("env", env, 50);
		// show("magnitudes", magnitudes, 50);
		selectedBeat = NO_BEAT_SELECTED;
		if (onsets.length == 0)
			minimumTime = maximumTime = 0;
		else {
			minimumTime = onsets[0];
			if ((env.length == 0) || ((mode & SHOW_MIDI) != 0))
				maximumTime = offsets[offsets.length - 1];
			else
				maximumTime = env[env.length - 1];
			if (beats.length != 0) {
				if (minimumTime > beats[0])
					minimumTime = beats[0];
				if (maximumTime < beats[beats.length-1])
					maximumTime = beats[beats.length-1];
			}
		}
		minimumTime = Math.floor(minimumTime - 0.5);
		maximumTime = Math.ceil(maximumTime + 0.5);
		if (resetSelection || (endSelection > maximumTime) ||
							(startSelection > maximumTime) ||
							(visibleTimeStart > maximumTime) ||
							(visibleTimeStart < minimumTime)) {
			startSelection = endSelection = -1.0;
			visibleTimeStart = minimumTime;
			visibleTimeLength = 5.0;
		}
		if (resetSelection) {
			yTop = 10 + metrics.getAscent();
			// if ((mode & SHOW_IBI) != 0) yTop += metrics.getAscent();
			yMid = yTop + 360;
			// if ((mode & (SHOW_SPECTRO | SHOW_MIDI)) != 0) yMid += 360;
			yBottom = yMid + 150;
			// if ((mode & SHOW_AUDIO) != 0) yBottom += 150;
			ySize = yBottom + 30;
			xSize = 1000;
			setPreferredSize(new Dimension(xSize, 560));
		}
	} // init()

	public void setMode(int on, int off) { mode |= on; mode &= ~0 - off; }

	synchronized public int getMinimum() {
		return (int) (minimumTime * 1000.0);
	}

	synchronized public int getMaximum() {
		return (int) (maximumTime * 1000.0);
	}

	synchronized public int getValue() {
		return (int) (visibleTimeStart * 1000.0);
	}

	synchronized public void setValue(int value) {
		visibleTimeStart = (double)value / 1000.0;
		repaint();
	}

	synchronized public int getVisibleAmount() {
		return (int) (visibleTimeLength * 1000.0);
	}

	synchronized public String getSelectedRegion(String prefix) {
		if (startSelection < 0)
			return prefix;
		if (prefix.length() > 0)
			prefix += ' ';
		prefix += "selectedStart=" + startSelection;
		if (endSelection >= 0)
			prefix += " selectedLength=" + (endSelection - startSelection);
		return prefix;
	}

	synchronized public void setVisibleAmount(int amount) {
		visibleTimeLength = (double)amount / 1000.0;
		repaint();
	}

	synchronized public void paintComponent(Graphics g) {
		xSize = getWidth();
		ySize = getHeight();
		paintBackground(g);
		paintAxes(g);
		if ((mode & SHOW_MIDI) != 0)
			paintMidiData(g);
		if ((mode & SHOW_AUDIO) != 0)
			paintAudioData(g);
		if ((mode & SHOW_SPECTRO) != 0)
			paintSpectroData(g);
		paintBeats(g);
	} // paintComponent()

	synchronized public void paintBackground(Graphics g) {
		setBackground(Color.white);
		super.paintComponent(g);
		g.setPaintMode();
		if (startSelection >= 0) {
			int start, end;
			if (startSelection <= endSelection) {
				start = timeToLocation(startSelection);
				end = timeToLocation(endSelection);
			} else if (endSelection < 0) {
				start = timeToLocation(startSelection);
				end = xSize;
			} else {
				end = timeToLocation(startSelection);
				start = timeToLocation(endSelection);
			}
			if (start > xSize)
				return;
			else if (start < 0)
				start = 0;
			if (end < 0)
				return;
			else if (end > xSize)
				end = xSize;
			g.setColor(Color.gray);
			g.fillRect(start, yMid, end-start+1, 151);
			// g.fillRect(start, yTop, end-start+1, yBottom-yTop+1);
		}
	} // paintBackground()

	synchronized public void paintAxes(Graphics g) {
		g.setColor(Color.black);
		g.drawLine(0,yBottom,xSize,yBottom);
		double tickGap = Math.ceil(visibleTimeLength / 7.5);
		if (tickGap <= 1.0)
			tickGap = 1.0 / Math.ceil(7.5 / visibleTimeLength);
		double ticks = Math.ceil(visibleTimeStart / tickGap) * tickGap;
		for ( ; ticks < visibleTimeStart + visibleTimeLength; ticks += tickGap){
			String label = Double.toString(ticks);
			int position = label.indexOf('.');
			if (position < 0) {
				position = label.length();
				label += ".";
			}
			label += "000";
			label = label.substring(0, position + (tickGap < 0.5 ? 3 : 2));
			position = timeToLocation(ticks) - metrics.stringWidth(label) / 2;
			if ((position>0) && (position+metrics.stringWidth(label) < xSize)) {
				g.drawString(label, position, yBottom + metrics.getAscent() +5);
				position = timeToLocation(ticks);
				g.drawLine(position, yBottom-5, position, yBottom+5);
			}
		}
	} // paintAxes()

	synchronized public void paintBeats(Graphics g) {
		g.setColor(Color.red);
		int xLocation, prevLocation = 0;
		for (int i=0; i<beats.length; i++) {
			xLocation = timeToLocation(beats[i]);
			if (xLocation < 0)
				continue;
			if (xLocation > xSize)
				break;
			if ((mode & SHOW_BEATS) != 0) {
				if (selectedBeat == i)
					g.drawRect(xLocation-1, 0, 2, yBottom-1);
				else
					g.drawLine(xLocation, 0, xLocation, yBottom-1);
			}
			if (((mode & SHOW_IBI) != 0) && (prevLocation != 0)) {
				// show inter-beat intervals (in msec)
				int xd = (int)(1000.0 * (locationToTime(xLocation) -
									 locationToTime(prevLocation)));
				String label = Integer.toString(xd);
				xd = (xLocation + prevLocation - metrics.stringWidth(label))/2;
				g.drawString(label, xd, 12);
			}
			prevLocation = xLocation;
		}
	} // paintBeats()

	synchronized public void paintMidiData(Graphics g) {
		g.setColor(Color.blue);
		int wd = (yMid - yTop) / (midiMax - midiMin + 1);
		for (int i=0; i<onsets.length; i++) {
			if ((pitches[i] < midiMin) || (pitches[i] > midiMax))
				continue;
			int xNoteOn = timeToLocation(onsets[i]);
			int xNoteOff = timeToLocation(offsets[i]);
			if (xNoteOff < 0)
				continue;
			if (xNoteOn > xSize)
				break;
			xNoteOff -= xNoteOn + 1;	// convert to offset
			if ((mode & SHOW_ONSETS) != 0) {
				g.drawLine(xNoteOn, yBottom, xNoteOn, yBottom-100);
				g.drawLine(xNoteOn-1, yBottom-5, xNoteOn+1, yBottom-5);
			}
			if ((mode & SHOW_DATA) != 0)
				g.fillRect(xNoteOn,wd*(midiMax-pitches[i]),xNoteOff,2);
		}
	} // paintMidiData()

	synchronized public void paintAudioData(Graphics g) {
		g.setColor(Color.green);
		int ht = 1; // (yBottom - yMid) / 150;
		int xPrev = env.length == 0? 0: timeToLocation(env[0]);
		for (int i = 0, j = 0; i < env.length; i++) {
			int xLocation = timeToLocation(env[i]);
			if (xLocation < 0)
				continue;
			if (xLocation > xSize)
				break;
			int yHi = yBottom - ht * magnitudes[i];
			if (yHi < yMid + 5)
				yHi = yMid + 5;
			if ((mode & SHOW_DATA) != 0) {
				if (xPrev < xLocation - 1)
					g.fillRect(xPrev+1, yHi, xLocation-xPrev, yBottom - yHi);
				else
					g.drawLine(xLocation, yHi, xLocation, yBottom-1);
			}
			while ((j < onsets.length) &&
						(timeToLocation(onsets[j]) <= xPrev))
				j++;
			while ((j < onsets.length) &&
						(timeToLocation(onsets[j]) <= xLocation)) {
				g.setColor(Color.blue);
				if ((mode & SHOW_ONSETS) != 0)
					g.fillRect(timeToLocation(onsets[j])-1, yHi-20, 3, 20);
				g.setColor(Color.green);
				j++;
			}
			xPrev = xLocation;
		}
	} // paintAudioData()
	
	synchronized public void paintSpectroData(Graphics g) {
		int sizeT = spectro.length / pitchCount;
		for (int i = 0; i < sizeT; i++) {
			int x1 = timeToLocation(tInc * (i + overlap / 2));
			if (x1 < 0)
				continue;
			if (x1 > xSize)
				break;
			int xd = timeToLocation(tInc * (i + overlap / 2 + 1)) - x1;
			int y1 = yMid;
			int yd = (y1 - yTop) / pitchCount; 
			for (int pitch = 0; pitch < pitchCount; pitch++) {
				g.setColor(getColour(spectro[pitch * sizeT + i]));
				y1 -= yd;
				g.fillRect(x1, y1, xd, yd);
			}
		}
	} // paintSpectroData()

	synchronized int timeToLocation(double time) {
		return (int)((time-visibleTimeStart)/visibleTimeLength*(double)xSize);
	}

	synchronized double locationToTime(int loc) {
		double tmp = (double)loc/(double)xSize * visibleTimeLength;
		if (tmp + visibleTimeStart < 0)
			return 0;
		return tmp + visibleTimeStart;
	}

	// we won't do this very often, and the array won't be too big, so ...
	synchronized double[] remove(int index, double[] arr) {
		double[] newArray = new double[arr.length-1];
		for (int i=0; i<index; i++)
			newArray[i] = arr[i];
		for (int i=index; i<newArray.length; i++)
			newArray[i] = arr[i+1];
		return newArray;
	}

	// we won't do this very often, and the array won't be too big, so ...
	synchronized double[] add(double value, int index, double[] arr) {
		double[] newArray = new double[arr.length+1];
		for (int i=0; i<index; i++)
			newArray[i] = arr[i];
		newArray[index] = value;
		for (int i=index+1; i<newArray.length; i++)
			newArray[i] = arr[i-1];
		return newArray;
	}

	synchronized int closestBeat(int x, boolean nextHighest) {
		int index=0;
		double time = locationToTime(x);
		while ((index < beats.length) && (time > beats[index]))
			index++;
		if (nextHighest)
			return index;
		if (index == beats.length)
			return index - 1;
		if (index == 0)
			return 0;
		if (Math.abs(time - beats[index]) <= Math.abs(time - beats[index-1]))
			return index;
		return index - 1;
	}
	
	synchronized int getBeatIndex(int x) {
		int index = closestBeat(x, false);
		if ((index>=0) && (Math.abs(x-timeToLocation(beats[index])) <= 5))
			return index;
		return NO_BEAT_SELECTED;
	}

	synchronized void press(int x, int y, MouseEvent e) {
		if ((y > 0) && (y < yBottom - 5)) {
			if (SwingUtilities.isMiddleMouseButton(e)) {
				selectedBeat = closestBeat(x, true);
				beats = add(locationToTime(x), selectedBeat, beats);
			} else {
				selectedBeat = getBeatIndex(x);
			}
		} else if (Math.abs(y - yBottom) < 5) {
			selectedBeat = REGION_SELECTED;
			if (SwingUtilities.isLeftMouseButton(e)) {
				if (e.isShiftDown() && (startSelection >= 0))
					endSelection = locationToTime(x);
				else {
					startSelection = locationToTime(x);
					endSelection = -1.0;
				}
			} else
				startSelection = endSelection = -1.0;
		}
		repaint();
	} // press()

	synchronized void release(int x, int y, MouseEvent e) {
		if ((selectedBeat >= 0) && (x >= 0) && (x <= xSize) &&
									(y >= 0) && (y <= ySize)) {
			if (SwingUtilities.isRightMouseButton(e))
				beats = remove(selectedBeat, beats);
			else {
				double time = locationToTime(x);
				while ((selectedBeat > 0) && (time < beats[selectedBeat-1])) {
					beats[selectedBeat] = beats[selectedBeat-1];
					selectedBeat--;
				}
				while ((selectedBeat < beats.length - 1) &&
								(time > beats[selectedBeat+1])) {
					beats[selectedBeat] = beats[selectedBeat+1];
					selectedBeat++;
				}
				beats[selectedBeat] = time;
			}
		} else if (selectedBeat == REGION_SELECTED) {
			if ((startSelection >= 0) && (x >= 0) && (x <= xSize) &&
							SwingUtilities.isLeftMouseButton(e)) {
				if ((e.isShiftDown() && (startSelection >= 0)) ||
							(Math.abs(timeToLocation(startSelection) - x) > 5))
					endSelection = locationToTime(x);
				else {
					startSelection = locationToTime(x);
					endSelection = -1.0;
				}
			}
		}
		selectedBeat = NO_BEAT_SELECTED;
		repaint();
	} // release()

	synchronized double[] getBeats() {
		return beats;
	}

	synchronized double[] getOnsets() {
		return onsets;
	}

	synchronized double[] getOffsets() {
		return offsets;
	}

	synchronized int[] getPitches() {
		return pitches;
	}

	synchronized public void setOnsets(double[] onsets) {
		this.onsets = onsets;
	}

	synchronized public void setOffsets(double[] offsets) {
		this.offsets = offsets;
		// setMode(SHOW_MIDI, SHOW_AUDIO | SHOW_SPECTRO);
	}

	synchronized public void setEnvTimes(double[] envTimes) {
		this.env = envTimes;
		// setMode(SHOW_AUDIO, SHOW_MIDI);
	}

	synchronized public void setMagnitudes(int[] magnitudes) {
		this.magnitudes = magnitudes;
	}

	synchronized public void setPitches(int[] pitches) {
		this.pitches = pitches;
	}

	synchronized public void setBeats(double[] beats) {
		this.beats = beats;
	}

	synchronized public void setSpectro(double[] spectro, int dim, double tInc,
					double loThres, double hiThres, double overlap){
        // setMode(SHOW_AUDIO | SHOW_SPECTRO, SHOW_MIDI);
        pitchCount = dim;
        this.tInc = tInc;
		this.spectro = spectro;
		this.loThreshold = loThres;
		this.hiThreshold = hiThres;
		this.overlap = overlap;
	}

} // class BeatTrackDisplay

class BTDMouseListener extends MouseAdapter
					   implements MouseListener, MouseMotionListener {

	BeatTrackDisplay btd;
	
	BTDMouseListener(BeatTrackDisplay btd) {
		this.btd = btd;
	}
	
	public void mousePressed(MouseEvent e) {
		btd.press(e.getX(), e.getY(), e);
	}

	public void mouseReleased(MouseEvent e) {
		btd.release(e.getX(), e.getY(), e);
	}

	public void mouseMoved(MouseEvent e) {
	}

	public void mouseDragged(MouseEvent e) {
		if (btd.selectedBeat == btd.REGION_SELECTED) {
			btd.release(e.getX(), e.getY(), e);
			btd.selectedBeat = btd.REGION_SELECTED;
		}
	}

} // class BTDMouseListener
