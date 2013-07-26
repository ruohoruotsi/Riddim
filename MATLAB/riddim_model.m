function ret = riddim_model(commandStr, entryStr, dataValue)

% declare persistent structures and variables
persistent handleStruct;
persistent dataStruct;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% init unused arguments
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if nargin < 3
  dataValue = [];
end
if nargin < 2
  entryStr = '';
end
if nargin == 0
  commandStr = 'init';
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% fprintf('commandStr %s entryStr %s\n',commandStr,entryStr); % debug
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% this gets called from riddim_tool.m on startup 
% init structure of figure handles and data items
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if strcmp(commandStr,'init')
  
  % do some init stuff, if need be . . so far no need.
  handleStruct.figure = findobj('tag','riddim_figure');   
 
  % 
  % save structure for various data later on.
  %
  s = struct('src',[],'out',[],'onsetinfo',[]);
  setappdata(handleStruct.figure, 'UserData', s);

  
  %
  % needed for the global mode situation
  %
  g = findobj('Tag', 'ModeFrame');
  currentMode = '';
  setappdata(g ,'UserData', currentMode) 

  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % read interface data    
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(commandStr,'get')
  ret = local_get_dataStruct_entry(dataStruct,entryStr);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % set interface data    
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(commandStr,'set')
  dataStruct = local_set_interface_entry(handleStruct,dataStruct,entryStr,dataValue);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % calls from gui    
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(commandStr,'gui')
  dataStruct = local_handle_gui_event(handleStruct, dataStruct, entryStr);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % return internal data to main tool, empty persistent variables   
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(commandStr,'return')
  ret          = dataStruct;
  dataStruct   =[];
  handleStruct =[];
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %%%%% detect invalid commandStr
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
else 
  warning('Sorry Sucka!!, command switch was not recognized');
end

 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 % catch GUI events
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function  dataStruct = local_handle_gui_event(handleStruct, dataStruct, entryStr)

% fprintf('entryStr %s\n', entryStr); % debug

% event of control1
if strcmp(entryStr,'my_gui_control1')
  
  % read gui/dataStruct...
  dataStruct = local_set_dataStruct_entry(handleStruct,dataStruct,'my_entry1',my_value1);
  
  % event of control2
elseif strcmp(entryStr,'my_gui_control2')
  
  % read gui/dataStruct...
  dataStruct = local_set_dataStruct_entry(handleStruct,dataStruct,'my_entry2',my_value2);
  
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Event handling for modal buttons, home, separate, get onsets, quit
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

elseif strcmp(entryStr,'gui_gohome')
  % IO todo need to clear out preexisting plots
  setGUIMode('home');
  set(handleStruct.figure,'Color', [ 0.354901960784314 0.13921568627451 0.13921568627451 ]);
  %[ 0.75294117647 0.52970588 0.52941176470588 ]);
  %[ 0.454901960784314 0.23921568627451 0.23921568627451 ]);
  
elseif strcmp(entryStr,'gui_separate')
  setGUIMode('separate');
  set(handleStruct.figure,'Color', [ 0.059803921568627 0.366666666666667 0.366666666666667 ]);
  %[ 0.176470588 0.752941176470588 0.752941176470588 ]);
  %[ 0.109803921568627 0.466666666666667 0.466666666666667 ]);
  
  
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Interpretation mode ... figuring out the lowest level pulse
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
elseif strcmp(entryStr,'gui_interpret')

  set(handleStruct.figure,'Color', [ 0.115686274509804 0.115686274509804 0.335294117647059 ]);
  setGUIMode('interpret');
  
  % get stored information, gkc_out & onset info
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  gkc_out = s.out;
  
  % set Visible buttons and drop down menu with size(gkc_out, 1)
  numMenuItems = size(gkc_out, 1);

  str = [];
  for i = 1:numMenuItems
    str = [str '|Stream ' num2str(i)]
  end
  
  h = findobj('Tag', 'LowestPulseMenu');
  set(h, 'String', str);
  set(h, 'Value', 1);
  val = get(h, 'Value')
  
  
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % QUIT
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

elseif strcmp(entryStr,'gui_destroy')
  close(handleStruct.figure);

  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Event handling for inter-modal buttons
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Event Handling for LOAD WAVE Button
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_loadwave')
  
  [fileName, pathName] = uigetfile('*.wav', 'Choose a WAV file to analyse');
  inputFile = wavread(strcat(pathName, fileName));
  
  % save input source to figure's UserData structure
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  s.src = inputFile;
  setappdata(g, 'UserData', s);


  h = findobj('Tag','SeparatedAxes');
  set(h, 'Visible', 'on');
  %  subplot('Position',[ 0.183783783783784 0.5 0.8 0.46969696969697 ]);
  %subplot('position',[.25 .55 .7 .4]);  
  subplot('position',[.23 .45 .74 .5]); plot(inputFile); axis tight;
  axis tight;
  h = gca;  
  set(h, 'Color', [0 0 0]);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Event Handling for PLAY WAVE Button
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_playWave')
  
  % save input source to figure's UserData structure
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  inputFile = s.src;
  
  % a double click is necessary . . .
  sound(inputFile, 16000)

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Event Handling for ISA Separation
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_kcdo')
  
  % IO todo need to make wavs clickable
  % IO get samplerate, should let in multirate wavs . . .
  sampleRate = 16000;
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  gkc_src = s.src;
  
  % number of components
  gkc_cmp = eval( ['[' get( findobj('Tag', 'NumComponentsEditText'), 'String') ']']);
  
  % analysis method
  switch get( findobj('Tag', 'Step1PopupMenu'), 'Value')
    case 1
      gkc_mth{1} = 'pca';
    case 2
      gkc_mth{1} = 'gca';
    case 3
      gkc_mth{1} = 'fastica';
    case 4
      gkc_mth{1} = 'fasticad';
    case 5
      gkc_mth{1} = 'jade';
  end
  
  switch get( findobj('Tag', 'Step2PopupMenu'), 'Value')
    case 1
      gkc_mth{2} = 'jade';
    case 2
      gkc_mth{2} = 'fastica';
    case 3
      gkc_mth{2} = 'fasticad';
    case 4
      gkc_mth{2} = 'pca';
    case 5
      gkc_mth{2} = 'null';
  end

  gkc_size = str2num( get( findobj('Tag', 'SizeEditText'), 'string'));
  gkc_hop  = str2num( get( findobj('Tag', 'HopEditText'), 'string'));
  gkc_pad  = str2num( get( findobj('Tag', 'PadEditText'), 'string'));
  
  % window types 1,2,
  switch get( findobj('Tag', 'FTWindowPopupMenu'), 'Value')
    case 1
      gkc_wn1 = hanning( gkc_size);
    case 2
      gkc_wn1 = ones( 1, gkc_size);
    case 3
      gkc_wn1 = kaiser( gkc_size, 3);
    case 4
      gkc_wn1 = kaiser( gkc_size, 8);
  end
  
  switch get( findobj('Tag', 'IFTWindowPopupMenu'), 'Value')
    case 1
      gkc_wn2 = hanning( gkc_size);
    case 2
      gkc_wn2 = ones( 1, gkc_size);
    case 3
      gkc_wn2 = kaiser( gkc_size, 3);
    case 4
      gkc_wn2 = kaiser( gkc_size, 8);
  end
  
      
  % set projection transformation type
  gkc_type = get( findobj('Tag', 'TypePopupMenu'), 'Value') - 1; 

  
  % set gkc_cst
  i = get( findobj('Tag', 'SelectionPopupMenu'), 'Value'); 
  switch i
    case 1
      gkc_cst = 'maxvariance';
    case 2
      gkc_cst = 'minvariance';
    case 3
      gkc_cst = 'maxkurtosis';
    case 4
      gkc_cst = 'minkurtosis';
  end
  
  
  % set gkc_nlf
  i = get( findobj('Tag', 'FunctionPopupMenu'), 'Value');
  switch i
    case 1
      gkc_nlf = 'abs';
    case 2
      gkc_nlf = 'none';
  end

  % this is because were only interested in time domain signals
  gkc_dom = 't'; 

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % call the subroutine
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  [gkc_out, gkc_b1, gkc_b2] = kc(gkc_src, gkc_cmp, gkc_dom, gkc_mth, ...
      [gkc_size, gkc_hop, gkc_pad], gkc_wn1, gkc_wn2, 0, 0, gkc_type,...
      gkc_cst, gkc_nlf);			    
  
  % get the inputFile data to replot if need be
  inputFile = gkc_src;
  clear gkc_src;

  % set Visible buttons and drop down menu with size(gkc_out, 1)
  numMenuItems = size(gkc_out, 1);
  setGUIMode('separateView');

  % clear the axis, so that we can plot things freshly
  cla;
  
  str = [];
  for i = 1:numMenuItems
    str = [str '|Stream ' num2str(i)]
  end
  
  h = findobj('Tag', 'SeparatedPopupMenu');
  set(h, 'String', str);
  set(h, 'Value', 1);
  val = get(h, 'Value')
  
  % save extracted output data
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  s.out = gkc_out;
  setappdata(g, 'UserData', s);
  
  
elseif strcmp(entryStr,'gui_playSeparated')
  
  % find out which wav was clicked on, 
  i = get( findobj('Tag', 'SeparatedPopupMenu'), 'Value') - 1;

  % get extracted outputs
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  gkc_out = s.out;

  % subract the mean and do some normalization
  gkc_out(i,:) = gkc_out(i,:) - mean( gkc_out(i,:));
  m = 1.1 * max( abs( gkc_out(i,:)));
  gkc_out(i,:) = gkc_out(i,:)/m;
  sound(gkc_out(i,:), 16000);
  
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % get Onsets
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_getOnsets')

  % find out which wav was clicked on, 
  i = get( findobj('Tag', 'SeparatedPopupMenu'), 'Value') - 1;

  % get extracted outputs
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  gkc_out = s.out;
  
  % take it up to 48000 
  y = interp(gkc_out(i,:), 3);
  subplot('position',[.23 .45 .74 .5]);  plot(y); axis tight;
  set(gca, 'Color', [0 0 0]);
  
  % set up grids . . .
  mininterval  = str2num( get( findobj('Tag', 'MinintervalEditText'), 'string'));
  thresholdValue = str2num( get( findobj('Tag', 'ThresholdValueEditText'), 'string'));

  [times, louds] = plotOnsets(y, mininterval, thresholdValue);

  % save onset info for use in persisting to a audio or MIDI file.
  onsetinfo = struct('times', times, 'louds', louds); 
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  s.onsetinfo = onsetinfo;
  setappdata(g, 'UserData', s);
  
  set(gca, 'XTick', times);
  set(gca, 'XColor', [1 0 0]);
  set(gca, 'XTickLabel', []);
  grid on;
  set(gca, 'YGrid', 'off');
  set(gca, 'Color', [0 0 0]);

  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Save Onsets as Audio, choose a wave to render with
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_saveOnsetsAudio')

  [fileName, pathName] = uigetfile('*.wav', 'Choose a WAV file with which to render pattern');
  inputFile = wavread(strcat(pathName, fileName));

  % find out which wav was clicked on, 
  i = get( findobj('Tag', 'SeparatedPopupMenu'), 'Value') - 1;
  disp(cat(2, 'saving audio as Riddim-stream-number-', num2str(i)));

 % get saved timing and loudness data
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  onsetinfo  = s.onsetinfo;
  times = onsetinfo.times;
  louds = onsetinfo.louds;

  threshold = str2num( get( findobj('Tag', 'ThresholdValueEditText'), 'string'));
  saveRiddimPatternAudio(times, louds, strcat(pathName, fileName), ...
      cat(2, 'Riddim-stream-number-',num2str(i)), threshold)
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Save Onsets as MIDI
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_saveOnsetsMIDI')
  disp('saving onsets as MIDI');
  
  % find out which wav was clicked on, 
  i = get( findobj('Tag', 'SeparatedPopupMenu'), 'Value') - 1;
  disp(cat(2, 'saving MIDI file as Riddim-MIDI-stream-number-', num2str(i)));
  
  % get saved timing and loudness data
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  onsetinfo  = s.onsetinfo;
  times = onsetinfo.times;
  louds = onsetinfo.louds;

  threshold = str2num( get( findobj('Tag', 'ThresholdValueEditText'), 'string'));
  saveRiddimPatternMIDI(times, louds, cat(2, 'Riddim-MIDI-stream-number-',num2str(i)),threshold);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % display the lowest level pulse for the audience to appreciate...
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
elseif strcmp(entryStr,'gui_lowestPulse')
  
  % find out which wav was clicked on, 
  i = get( findobj('Tag', 'LowestPulseMenu'), 'Value') - 1;

  % get stored information, gkc_out & onset info
  g = findobj('tag','riddim_figure');   
  s = getappdata(g, 'UserData');
  gkc_out = s.out;

  % take it up to 48000 
  y = interp(gkc_out(i,:), 3);
  subplot('position',[.3 .55 .65 .4]);  plot(y); axis tight;
  set(gca, 'Color', [0 0 0]);
  
  mininterval = 1/16;
  thresholdValue = .2;
  
  % set up grids . . .
  mininterval  = str2num( get( findobj('Tag', 'PulseMinintervalEditText'), 'string'));
  thresholdValue = str2num( get( findobj('Tag', 'PulseThresholdValueEditText'), 'string'));
  [times, louds] = plotOnsets(y, mininterval, thresholdValue);

  set(gca, 'XTick', times);
  set(gca, 'XColor', [1 0 0]);
  set(gca, 'XTickLabel', []);
  grid on;
  set(gca, 'YGrid', 'off');
  set(gca, 'Color', [0 0 0]);
  title('extracted waveform and onset points','Color',[1 1 1]);

  
  g = findobj('tag', 'LowestPulseAxes');
  tatumVector = getMetricalGrid(times, 48000);

  subplot('position',[.3 .07 .65 .4]); plot(tatumVector, ':');
  set(gca, 'Color', [0 0 0]);
  set(gca, 'XColor', [1 1 1]);
  set(gca, 'YColor', [1 1 1]);
  set(gca, 'XLabel', text('String', 'indicies indicate the number of accumulated onsets'));
  set(gca, 'YLabel', text('String', 'value lowest level pulse in SECONDS'));
  title('trajectories of lowest level pulse for this stream','Color',[1 1 1]);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % detect invalid entry string
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
else
  warning('Sorry, entry switch was not recognized, no action performed');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% keep track of which mode's pane is up and swap in and out the
% one being passed in
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [] = setGUIMode(new_mode)

g = findobj('Tag', 'ModeFrame');
currentMode = getappdata(g, 'UserData')
new_mode

% remove current mode;
switch currentMode
  case 'home'
    modeHomeOff;
  case 'separate'
    if ~strcmp(new_mode, 'separateView')
      modeSeparateOff;
    end
  case 'separateView'
    if ~(strcmp(new_mode, 'separateView') | ...
	  strcmp(new_mode, 'separate'))
      axis off;
      cla;	
    end
    modeSeparatedViewOnsetOff;
    modeSeparateOff;
  case 'interpret'
    modeInterpretOff;
    axis off;
    cla;
  case ''
  otherwise
    disp('Unknown mode to remove in SETGUIMode, what is going on??!!.')
end

% set up new mode
switch new_mode
  case 'home'
    modeHomeOn;
    setappdata(g,'UserData', 'home');
  case 'separate'
    modeSeparateOn;
    setappdata(g,'UserData', 'separate');
  case 'separateView'
    modeSeparatedViewOnsetOn;
    modeSeparateOn;
    setappdata(g,'UserData', 'separateView');
  case 'interpret'
    modeInterpretOn;
    setappdata(g,'UserData', 'interpret');
  otherwise
    disp('Unknown new mode to set SETGUIMode, what is going on??!!.')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Home mode

function [] = modeHomeOn()

 h = findobj('Tag', 'HomeModeFrame');
 set(h, 'Visible', 'on');
 h = findobj('Tag', 'riddim_loadWave');
 set(h, 'Visible', 'on');
 h = findobj('Tag', 'riddim_playWave');
 set(h, 'Visible', 'on');
 
function [] = modeHomeOff()

 h = findobj('Tag', 'HomeModeFrame');
 set(h, 'Visible', 'off');
 h = findobj('Tag', 'riddim_loadWave');
 set(h, 'Visible', 'off');
 h = findobj('Tag', 'riddim_playWave');
 set(h, 'Visible', 'off');

%% Separate Mode
 
function [] = modeSeparateOn()

h = findobj('Tag', 'SeparateModeFrame');
set(h, 'Visible', 'on');

h = findobj('Tag','Step1PopupMenu');
set(h, 'Visible', 'on');

h = findobj('Tag','Analysis1Text');
set(h, 'Visible', 'on');

h = findobj('Tag','TypePopupMenu');
set(h, 'Visible', 'on');

h = findobj('Tag','TypeText');
set(h, 'Visible', 'on');

h = findobj('Tag','SizeEditText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','SizeText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','HopEditText');  
set(h, 'Visible', 'on');

h = findobj('Tag','HopText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','PadEditText');  
set(h, 'Visible', 'on');

h = findobj('Tag','SelectionPopupMenu');  
set(h, 'Visible', 'on');

h = findobj('Tag','PadText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','SelectionText');  
set(h, 'Visible', 'on');

h = findobj('Tag','Step2Text'); 
set(h, 'Visible', 'on');

h = findobj('Tag','Step2PopupMenu');  
set(h, 'Visible', 'on');

h = findobj('Tag','NumComponentsText');  
set(h, 'Visible', 'on');

h = findobj('Tag','NumComponentsEditText');  
set(h, 'Visible', 'on');

h = findobj('Tag','FTWindowText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','FTWindowPopupMenu');  
set(h, 'Visible', 'on');

h = findobj('Tag','IFTWindowPopupMenu'); 
set(h, 'Visible', 'on');

h = findobj('Tag','FunctionPopupMenu'); 
set(h, 'Visible', 'on');

h = findobj('Tag','IFTWindowText'); 
set(h, 'Visible', 'on');

h = findobj('Tag','FunctionText');  
set(h, 'Visible', 'on');

h = findobj('Tag','doAnalysisButton');  
set(h, 'Visible', 'on');


function [] = modeSeparateOff()

h = findobj('Tag', 'SeparateModeFrame');
set(h, 'Visible', 'off');

h = findobj('Tag', 'SeparateModeFrame');
set(h, 'Visible', 'off');

h = findobj('Tag','Step1PopupMenu');
set(h, 'Visible', 'off');

h = findobj('Tag','Analysis1Text');
set(h, 'Visible', 'off');

h = findobj('Tag','TypePopupMenu');
set(h, 'Visible', 'off');

h = findobj('Tag','TypeText');
set(h, 'Visible', 'off');

h = findobj('Tag','SizeEditText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','SizeText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','HopEditText');  
set(h, 'Visible', 'off');

h = findobj('Tag','HopText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','PadEditText');  
set(h, 'Visible', 'off');

h = findobj('Tag','SelectionPopupMenu');  
set(h, 'Visible', 'off');

h = findobj('Tag','PadText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','SelectionText');  
set(h, 'Visible', 'off');

h = findobj('Tag','Step2Text'); 
set(h, 'Visible', 'off');

h = findobj('Tag','Step2PopupMenu');  
set(h, 'Visible', 'off');

h = findobj('Tag','NumComponentsText');  
set(h, 'Visible', 'off');

h = findobj('Tag','NumComponentsEditText');  
set(h, 'Visible', 'off');

h = findobj('Tag','FTWindowText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','FTWindowPopupMenu');  
set(h, 'Visible', 'off');

h = findobj('Tag','IFTWindowPopupMenu'); 
set(h, 'Visible', 'off');

h = findobj('Tag','FunctionPopupMenu'); 
set(h, 'Visible', 'off');

h = findobj('Tag','IFTWindowText'); 
set(h, 'Visible', 'off');

h = findobj('Tag','FunctionText');  
set(h, 'Visible', 'off');

h = findobj('Tag','doAnalysisButton');  
set(h, 'Visible', 'off');

% View the Separated Files and get Onsets Mode

function [] = modeSeparatedViewOnsetOn()

 h = findobj('Tag','SeparatedPopupMenu');
 set(h, 'Visible', 'on');

 h = findobj('Tag','SaveMidi');
 set(h, 'Visible', 'on');
 h = findobj('Tag','SaveAudio');
 set(h, 'Visible', 'on');

 h = findobj('Tag','PlaySeparatedButton');
 set(h, 'Visible', 'on');
  h = findobj('Tag','SeparatedViewText');
 set(h, 'Visible', 'on');
 h = findobj('Tag','SeparatedViewFrame');
 set(h, 'Visible', 'on');

 h = findobj('Tag','MinintervalText');
 set(h, 'Visible', 'on');
 h = findobj('Tag','MinintervalEditText');
 set(h, 'Visible', 'on');
 
 h = findobj('Tag','ThresholdText');
 set(h, 'Visible', 'on');
 h = findobj('Tag','ThresholdValueEditText');
 set(h, 'Visible', 'on');

 h = findobj('Tag','OriginalWave');
 set(h, 'Visible', 'on');
 
function [] = modeSeparatedViewOnsetOff()

 h = findobj('Tag','SeparatedPopupMenu');
 set(h, 'Visible', 'off');
 
 h = findobj('Tag','SaveMidi');
 set(h, 'Visible', 'off');
 h = findobj('Tag','SaveAudio');
 set(h, 'Visible', 'off');
 
 h = findobj('Tag','PlaySeparatedButton');
 set(h, 'Visible', 'off');
 h = findobj('Tag','SeparatedViewText');
 set(h, 'Visible', 'off');
 h = findobj('Tag','SeparatedViewFrame');
 set(h, 'Visible', 'off');

 
 h = findobj('Tag','MinintervalText');
 set(h, 'Visible', 'off');
 h = findobj('Tag','MinintervalEditText');
 set(h, 'Visible', 'off');

 h = findobj('Tag','ThresholdText');
 set(h, 'Visible', 'off');
 h = findobj('Tag','ThresholdValueEditText');
 set(h, 'Visible', 'off');

 h = findobj('Tag','OriginalWave');
 set(h, 'Visible', 'off');
 
%% Interpretion Mode 
 
function [] = modeInterpretOn()

 h = findobj('Tag', 'InterpretModeFrame');
 set(h, 'Visible', 'on');

 % h = findobj('Tag', 'LowestPulseAxes');
 % set(h, 'Visible', 'on');
 
 h = findobj('Tag', 'LowestPulseMenu');
 set(h, 'Visible', 'on');

 h = findobj('Tag', 'LowestPulseText');
 set(h, 'Visible', 'on');
 
 h = findobj('Tag','PulseMinintervalText');
 set(h, 'Visible', 'on');
 h = findobj('Tag','PulseMinintervalEditText');
 set(h, 'Visible', 'on');

 h = findobj('Tag','PulseThresholdText');
 set(h, 'Visible', 'on');
 h = findobj('Tag','PulseThresholdValueEditText');
 set(h, 'Visible', 'on');
 
function [] = modeInterpretOff()

h = findobj('Tag', 'InterpretModeFrame');
set(h, 'Visible', 'off');

h = findobj('Tag', 'LowestPulseAxes');
set(h, 'Visible', 'off');

h = findobj('Tag', 'LowestPulseMenu');
set(h, 'Visible', 'off');

h = findobj('Tag', 'LowestPulseText');
set(h, 'Visible', 'off');


 h = findobj('Tag','PulseMinintervalText');
 set(h, 'Visible', 'off');
 h = findobj('Tag','PulseMinintervalEditText');
 set(h, 'Visible', 'off');

 h = findobj('Tag','PulseThresholdText');
 set(h, 'Visible', 'off');
 h = findobj('Tag','PulseThresholdValueEditText');
 set(h, 'Visible', 'off');
