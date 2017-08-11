//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: ZoomListener.java
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

class ZoomListener implements ActionListener {

	JScrollBar scroller;
	BeatTrackDisplay displayPanel;
	JTextField valueField;

	public ZoomListener(BeatTrackDisplay btd, JScrollBar sb, JTextField vf) {
		displayPanel = btd;
		scroller = sb;
		valueField = vf;
		valueField.setText(Double.toString(btd.getVisibleAmount()/1000.0));
	} // ZoomListener constructor

	public static double delta(double value, double sign) {
		if (value >= 5.0)
			return Math.floor(value + sign * value / 5.0 + 0.5);
		if (value >= 1.0)
			return Math.floor(5.0 * (value + sign * value / 5.0) + 0.5) / 5.0;
		if (value >= 0.5)
			return Math.floor(10.0 * (value + sign * value / 5.0) + 0.5) / 10.0;
		return Math.floor(10.0 * (value + sign * 0.1) + 0.5) / 10.0;
	} // delta()

	public void actionPerformed(ActionEvent e) {
		double value = (double)displayPanel.getVisibleAmount() / 1000.0;
		try {
			String comm = e.getActionCommand();
			if (comm.equals("+"))
				value = delta(value, 1.0);
			else if (comm.equals("-"))
				value = delta(value, -1.0);
			else
				value = Double.parseDouble(valueField.getText());
			if (value < 0.1)
				value = 0.1;
			displayPanel.setVisibleAmount((int)(value * 1000.0));
			scroller.setVisibleAmount(displayPanel.getVisibleAmount());
			valueField.setText(Double.toString(value));
		} catch (NumberFormatException nfe) {
			valueField.setText("");
		}
	} // actionPerformed()

} // class ZoomListener
