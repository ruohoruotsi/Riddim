//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: Gui.java
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

public class Gui extends JFrame {

	JobQ jobQ;
	BeatTrackDisplay displayPanel;
	JScrollBar scroller;
	JPanel scrollPane;
	ControlPanel controlPanel;
	boolean ignoreWarnings;

	public Gui(String[] args) {
		super("BeatRoot " + " (version 0.0)");
		try {
			UIManager.setLookAndFeel(
				UIManager.getCrossPlatformLookAndFeelClassName());
		} catch (Exception e) { }
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				jobQ.add(JobQ.QUIT);
			}
		});
		ignoreWarnings = false;

		displayPanel = new BeatTrackDisplay(this);
		scroller = new JScrollBar();
		scroller.setMinimum(displayPanel.getMinimum());
		scroller.setMaximum(displayPanel.getMaximum());
		scroller.setValue(displayPanel.getValue());
		scroller.setVisibleAmount(displayPanel.getVisibleAmount());
		scroller.setUnitIncrement(100);
		scroller.setBlockIncrement(400);
		scroller.setOrientation(Adjustable.HORIZONTAL);
		scroller.setPreferredSize(new Dimension(1000, 17));
		scrollPane = new JPanel();
		scrollPane.setPreferredSize(new Dimension(1010, 587));
		scrollPane.setBackground(Color.black);
		scrollPane.add(displayPanel, BorderLayout.CENTER);
		scrollPane.add(scroller, BorderLayout.SOUTH);
		scroller.addAdjustmentListener(new PanelScroller(displayPanel));

		controlPanel = new ControlPanel(displayPanel, scroller, args[0]);
		jobQ = controlPanel.getJobQueue();

		JPanel pane = new JPanel();
		pane.setBackground(Color.black);
		pane.setLayout(new BorderLayout());
		pane.add(scrollPane, BorderLayout.CENTER);
		pane.add(controlPanel, BorderLayout.SOUTH);
		setContentPane(pane);
		pack();
		show();
	} // constructor

	public double[] getBeatData() {
		return displayPanel.getBeats();
	} // getData()

	public String getRequest() {
		return jobQ.remove();
	}

	public void setAudioData(double[] onsets, double[] envTimes, int[] envMags){
		displayPanel.setOnsets(onsets);
		displayPanel.setEnvTimes(envTimes);
		displayPanel.setMagnitudes(envMags);
		updateDisplay(true);
	} // setAudioData()

	public void setMidiData(double[] onsets, double[] offsets, int[] pitches) {
		displayPanel.setOnsets(onsets);
		displayPanel.setOffsets(offsets);
		displayPanel.setPitches(pitches);
		updateDisplay(true);
	} // setMidiData()

	public void setBeatData(double[] beats) {
		displayPanel.setBeats(beats);
		updateDisplay(false);
	} // setBeatData()

    public void setSpectroData(double[] data, int dim, double tInc,
							   double loThres, double hiThres, double overlap) {
        displayPanel.setSpectro(data, dim, tInc, loThres, hiThres, overlap);
        updateDisplay(true);
    } // setSpectroData()

	void updateDisplay(boolean resetSelection) {
		displayPanel.init(resetSelection);
		scroller.setMinimum(displayPanel.getMinimum());
		scroller.setMaximum(displayPanel.getMaximum());
		scroller.setValue(displayPanel.getValue());
		scroller.setVisibleAmount(displayPanel.getVisibleAmount());
		// scrollPane.setPreferredSize(new Dimension(1010,
		// 										  27+displayPanel.getHeight()));
		displayPanel.repaint();
		// setSize(new Dimension(getWidth(),
		// 					  scrollPane.getHeight()+controlPanel.getHeight()));
		// pack();
		// System.err.println("  Scroll:  " + scrollPane.getHeight() +
		// 				   "\n  Control: " + controlPanel.getHeight() +
		// 				   "\n  this:    " + getHeight());
	} // update()

	public void showMessage(String message) {
		// JOptionPane.showMessageDialog(this, "Warning: " + message);
		if (!ignoreWarnings) {
			String string1 = "OK";
			String string2 = "Ignore future warnings";
			Object[] options = {string1, string2};
			int n = JOptionPane.showOptionDialog(this, "Warning: " + message,
					"Warning", JOptionPane.YES_NO_OPTION,
					JOptionPane.WARNING_MESSAGE, null, options, string1);
			ignoreWarnings = (n != JOptionPane.YES_OPTION);
		}
	} // showMessage()

	public void setBlindMode(int mode, int blindFlags) {
		displayPanel.setMode(BeatTrackDisplay.DEFAULT_MODE | mode, blindFlags);
	} // setBlindMode()

	// test class implementation
	public static void main(String[] args) {
		Gui g = new Gui(args);
	} // main method for class Gui

} // class Gui
