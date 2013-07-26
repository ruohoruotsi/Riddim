function [peaktimes, peakpresses, peakBgPresses, bankPresses] = getOnsets(signal, srate,...
    mininterval, loudthreshs, maskLevs, lboardLen, diffPrecision, thresholdValue)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ---- Default parameter values
% Minimum length between note onsets in seconds (=60/tempo).
if ~exist( 'mininterval'),
  mininterval = 1/16;
end;

%
% Threshold for final selection of loud enough onsets
if ~exist( 'loudthreshs'),
  %  loudthreshs = -34;
  loudthreshs = -10;
end;


% Level of masking: mask level is <level before onset>-maskLev, and the
% onsetting sound gets level <rise of level> - <level before onset> - maskLev.
if ~exist( 'maskLevs'),
  maskLevs = -56;  % 19 dB below the masker
end;


% Locally adapting loudness window length (seconds, 2-8 s. is good)
if ~exist( 'lboardLen'),
  %lboardLen = 4; % in seconds  ---> 1,2,4,8,16
  lboardLen = 8;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% ---- Initializations
% Signal: remove DC component and normalize signal amplitude between -1 and 1
signal = signal(:).';
signal = signal - mean(signal);
signal = signal / max(abs(signal));

% Time constant: 1/timefrac seconds. Auditory limit is ~ 20 discrete events/s.
timefrac = 1/20;
timefracsamps = round(timefrac*srate);

% Overall loudness is estimated 'loudtimefrac' times per second over the signal
loudtimefrac = 5; % values > 4 are good, no big effect

% Reference intensity in dBs. Intesity of a sound is L = 20log10(I/Io) dB. 
Io = -150;

% Filterbank parameters (use rhytfiltbankcreate.m to compose these).
load onsetsBank; % Fbands, Bbank, Abank
bands = [Fbands(1:end-1)', Fbands(2:end)'];

% -Decimation filter. For the sake of efficiency the signal is decimated 
%  during the band-wise calculations. 
%  **NOTE: Decimation is done for
%  efficiency. Does NOT have to drop out all frequencies above, say 20 Hz.

Fdec = 2000; % formerly 100 % Hz. Decimation cutoff frequency
[Bdec, Adec] = cheby1( 6, 1, .5*Fdec/(srate/2)); % 6th order, x dB pass ripple

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% - Initializations
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bandid = 1;  

plot(signal);
[peaktimes, peakpresses, peakBgPresses, bankPresses] = ...
    mexBandOnsets(signal, srate, Io, timefrac, loudtimefrac, ...
    diffPrecision, Fdec, Bdec, Adec, thresholdValue);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% -PRUNE OUT peaks that are at same band and too close to each other.
%  Otherwise they may override their louder neighbours in the integration.
%     - Onsets must be in ascending time order, if C-version is used  

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% the io jazzy pruning peaktimes first, peakpresses next
%
ids = mexPruneOnsets( peaktimes, peakpresses,...
    min(peakpresses), mininterval*srate)';

%
% iroro's mexPruneOnsets returns column vectors rather than row
%

ids = ids';
peaktimes = peaktimes(ids);
peakpresses = peakpresses(ids);
peakBgPresses = peakBgPresses(ids);

%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%set(gca, 'XTick', peaktimes);
%set(gca, 'XColor', [1 0 0]);
%set(gca, 'XTickLabel', []);
%grid on;
%set(gca, 'YGrid', 'off');

return;

