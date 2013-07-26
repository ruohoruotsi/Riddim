function riddim = saveRiddimPatternMIDI(times, louds, outfilename, threshold)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SAVERIDDIMPATTERNMIDI takes a vector of onsets and write a MIDI file
%
% to be used by iroro to evaluate the quality of extracted riddimz 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% convert louds to MIDI velocities/volume
louds = ((louds + threshold)/max(louds))*127;

% convert times to milli - seconds
times = times/48000;  % seconds

% save as MIDI
mexSaveMIDI(times, louds, cat(2, outfilename, '.mid'));