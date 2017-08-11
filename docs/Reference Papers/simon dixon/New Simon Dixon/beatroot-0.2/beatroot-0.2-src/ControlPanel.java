//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: ControlPanel.java
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
import javax.swing.filechooser.FileFilter;
import java.io.File;

class ControlPanel extends JPanel {
	
	JobQ jobq;

	class MyFilter extends FileFilter {
		private String suffix;
		private String description;
		MyFilter(String suff, String desc) {
			suffix = suff;
			description = desc + " files (*" + suff + ")";
		}
		public boolean accept(File f) {
			return (f!=null) && (f.isDirectory()||f.getName().endsWith(suffix));
		}
		public String getDescription() {
			return description;
		}
	} // inner class MyFilter
		
	final MyFilter waveFileFilter = new MyFilter(".wav", "Wave");
	final MyFilter sndFileFilter = new MyFilter(".snd", "Sun sound");
	final MyFilter midiFileFilter = new MyFilter(".mid", "Midi");
	final MyFilter tmfFileFilter = new MyFilter(".tmf", "Midi text");
	final MyFilter matchFileFilter = new MyFilter(".match", "Score match");

	class DataChooser extends JFileChooser implements ActionListener {

		JLabel fileName;
		String pathName;
		BeatFileChooser bfc;
		SaveAllChooser sac;
		CutChooser cut;
		JobQ jobq;

		DataChooser(JLabel name, BeatFileChooser b, SaveAllChooser s,
					CutChooser c, JobQ j) {
			super(".");
			fileName = name;
			bfc = b;
			sac = s;
			cut = c;
			jobq = j;
			jobq.setChooser(this);
			addChoosableFileFilter(waveFileFilter);
			addChoosableFileFilter(sndFileFilter);
			addChoosableFileFilter(midiFileFilter);
			addChoosableFileFilter(tmfFileFilter);
			addChoosableFileFilter(matchFileFilter);
			addChoosableFileFilter(getAcceptAllFileFilter());
		}

		public String getFileName() {
			return pathName;
		}

		public void actionPerformed(ActionEvent e) {
			if (showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
				String name = getSelectedFile().getName();
				fileName.setText("File: " + name);
				bfc.setDefault(name);
				sac.setDefault(name);
				cut.setDefault(name);
				pathName = getSelectedFile().getAbsolutePath();
				jobq.actionPerformed(e);
			}
		}

	} // inner class DataChooser

	class CutChooser extends JFileChooser implements ActionListener {

		JobQ jobq;
		JTextField options;
		BeatTrackDisplay btd;
		String name;

		CutChooser(JobQ j, JTextField o, BeatTrackDisplay b) {
			super(".");
			jobq = j;
			options = o;
			btd = b;
			name = null;
			addChoosableFileFilter(tmfFileFilter);
			addChoosableFileFilter(getAcceptAllFileFilter());
		}

		public void setDefault(String d) {
			name = splitExtension(d, true) + "_1.tmf";
		}

		public void actionPerformed(ActionEvent e) {
			if (name != null)
				setSelectedFile(new File(name));
			double[] cutPoints = btd.getBeats();
			for (int i = 0; i <= cutPoints.length; i++) {
				double min = i > 0? cutPoints[i-1]: 0;
				double len = i < cutPoints.length? cutPoints[i] - min: -1;
				if (len == 0)
					continue;
				if (showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
					if (!getSelectedFile().exists() || (
						JOptionPane.showConfirmDialog(null,
						"File " + getSelectedFile().getAbsolutePath() +
						" exists.\nDo you want to replace it?", "Are you sure?",
						JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION))
					  jobq.add(e.getActionCommand() + ":" +
								getSelectedFile().getAbsolutePath() + ":" +
								options.getText() + " selectedStart=" + min +
								" selectedLength=" + len);
				} else
					break;
			}
		}

	} // inner class CutChooser

	class BeatFileChooser extends JFileChooser implements ActionListener {

		JobQ jobq;
		String dataFileName;
		JTextField options;
		BeatTrackDisplay btd;

		BeatFileChooser(JobQ j, JTextField o, BeatTrackDisplay b) {
			super(".");
			jobq = j;
			options = o;
			btd = b;
			dataFileName = null;
			addChoosableFileFilter(tmfFileFilter);
		}

		public void setDefault(String d) {
			dataFileName = splitExtension(d, true) + "-beatTrackData.tmf";
		}

		public void actionPerformed(ActionEvent e) {
			if (dataFileName != null) {
				setSelectedFile(new File(dataFileName));
				dataFileName = null;	// only use default first time
			}
			if (e.getActionCommand().startsWith("Load")) {
				if (showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
					jobq.add(e.getActionCommand() + ":" +
							 getSelectedFile().getAbsolutePath() + ":" +
							 btd.getSelectedRegion(options.getText()));
			} else if (e.getActionCommand().equals("Save Beats") ||
						showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
				if (!getSelectedFile().exists() || (
						JOptionPane.showConfirmDialog(null,
						"File " + getSelectedFile().getAbsolutePath() +
						" exists.\nDo you want to replace it?", "Are you sure?",
						JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION))
					jobq.add(e.getActionCommand() + ":" +
							getSelectedFile().getAbsolutePath() + ":" +
							btd.getSelectedRegion(options.getText()));
			}
		}

	} // inner class BeatFileChooser

	class SaveAllChooser extends BeatFileChooser implements ActionListener {

		SaveAllChooser(JobQ j, JTextField o, BeatTrackDisplay b) {
			super(j, o, b);
			addChoosableFileFilter(waveFileFilter);
			addChoosableFileFilter(sndFileFilter);
		}

		public void setDefault(String d) {
			dataFileName = splitExtension(d, true) + "+beats";
			if (splitExtension(d, false).equalsIgnoreCase(".mid") ||
					splitExtension(d, false).equalsIgnoreCase(".match") ||
					splitExtension(d, false).equalsIgnoreCase(".tmf")) {
				dataFileName += ".tmf";
				setFileFilter(tmfFileFilter);
			} else if (splitExtension(d, false).equalsIgnoreCase(".snd")) {
				dataFileName += ".snd";
				setFileFilter(sndFileFilter);
			} else if (splitExtension(d, false).equalsIgnoreCase(".wav")) {
				dataFileName += ".wav";
				setFileFilter(waveFileFilter);
			} else
				setFileFilter(getAcceptAllFileFilter());
		}

	} // inner class SaveAllChooser

	class ButtonPanel extends JPanel {
		public ButtonPanel(int r, int c) {
			super(new GridLayout(r,c));
		}
		public void addButton(String s, ActionListener al) {
			JButton b = new JButton(s);
			b.addActionListener(al);
			add(b);
		}
	} // inner class ButtonPanel

	public ControlPanel(BeatTrackDisplay displayPanel, JScrollBar scroller,
						String args) {
		setPreferredSize(new Dimension(1000, 100));
		setLayout(new BorderLayout());
		setBorder(BorderFactory.createLineBorder(Color.black, 5));

		JLabel fileName = new JLabel("File: <none>");
		JTextField argumentField = new JTextField(24);
		jobq = new JobQ(argumentField, displayPanel);

		ButtonPanel buttonPanel;
		if (args.endsWith(".tmf")) {
			final String saveComm = "Save Beats:" + args + ":";
			buttonPanel = new ButtonPanel(3,2);
			buttonPanel.addButton("New", jobq);
			buttonPanel.addButton("Play", jobq);
			buttonPanel.addButton("Clear", jobq);
			buttonPanel.addButton("Stop", jobq);
			buttonPanel.addButton("Save Beats", new ActionListener() {
			 	public void actionPerformed(ActionEvent e) {jobq.add(saveComm);}
			});
			// buttonPanel.addButton("Quit", new ActionListener() {
			// 	public void actionPerformed(ActionEvent e) {
			// 		jobq.add("Save Beats");
			// 		jobq.add(JobQ.QUIT);
			// 	}
			// });
		} else {
			BeatFileChooser beatChooser = new BeatFileChooser(jobq,
												argumentField, displayPanel);
			SaveAllChooser saveChooser = new SaveAllChooser(jobq, argumentField,
																displayPanel);
			CutChooser cutChooser = new CutChooser(jobq, argumentField,
																displayPanel);
			DataChooser fileChooser = new DataChooser(fileName, beatChooser,
												  saveChooser, cutChooser,jobq);

			Box optionsBox = new Box(BoxLayout.X_AXIS);
			optionsBox.add(new JLabel("Options: "));
			optionsBox.add(argumentField);

			JPanel viewPanel = new JPanel(new GridLayout(1,3));
			JLabel viewLabel = new JLabel("View range (s): ");
			JButton dec = new JButton("-");
			JTextField zoomField = new JTextField(5);
			zoomField.setActionCommand("text");
			JButton inc = new JButton("+");
			viewPanel.add(viewLabel);
			viewPanel.add(dec);
			viewPanel.add(zoomField);
			viewPanel.add(inc);
			ZoomListener viewListener = new ZoomListener(displayPanel, scroller,
															zoomField);
			inc.addActionListener(viewListener);
			zoomField.addActionListener(viewListener);
			dec.addActionListener(viewListener);

			JPanel fileAndOptions = new JPanel(new GridLayout(3,1));
			fileAndOptions.setBorder(BorderFactory.createEmptyBorder(0,5,0,0));
			fileAndOptions.add(fileName);
			fileAndOptions.add(optionsBox);
			fileAndOptions.add(viewPanel);
			add(fileAndOptions, BorderLayout.WEST);
			buttonPanel = new ButtonPanel(3,4);
			buttonPanel.addButton("New", fileChooser);
			buttonPanel.addButton("Re-track", jobq);
			buttonPanel.addButton("Save Beats", beatChooser);
			buttonPanel.addButton("Play", jobq);
			buttonPanel.addButton("Clear", jobq);
			buttonPanel.addButton("Undo", jobq);
			buttonPanel.addButton("Save Beats As", beatChooser);
			buttonPanel.addButton("Stop", jobq);
			buttonPanel.addButton("Load Beats", beatChooser);
			buttonPanel.addButton("Cut File", cutChooser);
			buttonPanel.addButton("Save All", saveChooser);
		}
		buttonPanel.addButton("Quit", new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (JOptionPane.showConfirmDialog(null, "Warning: " +
					"this operation may result in loss of data :)\n" + 
					"Do you really want to quit?", "Just checking ...",
					JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
						jobq.add(JobQ.QUIT);
			}
		});
		add(buttonPanel, BorderLayout.EAST);
	} // ControlPanel() constructor

	private String splitExtension(String s, boolean body) {
		int i = s.lastIndexOf(".");
		if (i < 0)
			return s;
		return body? s.substring(0,i): s.substring(i);
	} // splitExtension()

	public JobQ getJobQueue() {
		return jobq;
	} // getJobQueue()

} // class ControlPanel
