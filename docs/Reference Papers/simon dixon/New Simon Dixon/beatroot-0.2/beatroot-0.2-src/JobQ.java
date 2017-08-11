//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: JobQ.java
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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

class JobQ implements ActionListener {
	public static final String CANCEL = "_cancel_";
	public static final String QUIT = "_quit_";
	private static int nextRequestID = 0;
	private int requestID;
	private String request;
	private JobQ next;
	private ControlPanel.DataChooser chooser;
	private JTextField argField;
	private BeatTrackDisplay btd;

	private JobQ(String request) {
		requestID = nextRequestID++;
		this.request = request;
	}

	public JobQ(JTextField cf, BeatTrackDisplay b) {
		this("_dummy_request_for_head_of_queue_");
		chooser = null;
		argField = cf;
		btd = b;
	}

	public void setChooser(ControlPanel.DataChooser dc) {
		chooser = dc;
	}

	public synchronized void add(String newRequest) {
		if ((newRequest == QUIT) || (newRequest == CANCEL) || (next == null))
			next = new JobQ(newRequest);
		else
			next.add(newRequest);
	}

	public synchronized String remove() {
		if (next == null)
			return null;
		String s = next.request;
		next = next.next;
		return s;
	}

	public void actionPerformed(ActionEvent e) {
		add(e.getActionCommand() + ":" +
			((chooser != null)? chooser.getFileName():"") + ":" +
			btd.getSelectedRegion(argField.getText()));
	}

} // class JobQ
