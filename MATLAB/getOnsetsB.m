function [times, louds, types, peaktimes, peakpresses,...
      peakBgPresses,bankPresses] = getOnsets(signal, srate,...
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
%

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

set(gca, 'XTick', peaktimes);
set(gca, 'XColor', [1 0 0]);
set(gca, 'XTickLabel', []);
grid on;
set(gca, 'YGrid', 'off');


% ----- COMBINE ONSET COMPONENTS to yield global onsets -----------
%  -Sort collected peaks in time.
%   (In fact this would not be needed at the moment, but sorting is done to
%    make sure that modifications above do not create surprises)
%[peaktimes, ids] = sort( peaktimes);
%peakPressBids = peakPressBids(ids);
%peakpresses = peakpresses(ids);
%peakBgPresses = peakBgPresses(ids);

% Without this last peak would not be treated later when bands are combined.
% This is cancelled at a later time.
peaktimes = [peaktimes, 0];

% Put pressures to dB scale
peakpresses =   20*log10( peakpresses) - Io;
peakBgPresses = 20*log10( peakBgPresses) - Io;

% Initializations to zero
% onsetlouds = zeros( length(maskLevs), length(peaktimes));
onsetlouds = zeros( 1, length(peaktimes));
onsettimes = zeros( 1, length(peaktimes));
onsettypes = zeros( 1, length(peaktimes));
peakfut = 1;


for peakid = 1:length(peaktimes)-1,
  % SLIDING 'TIMEFRAC' TIME WINDOW, inside which onset comps. are integrated
  while peakfut < length(peaktimes)-1 &...
	peaktimes(peakfut+1)-peaktimes(peakid) < timefracsamps,
    peakfut = peakfut+1;
  end

  % INTEGRATED LOUDNESS IS FOUND by calculating loudness of the onset using
  % sound pressures of the peaks at different bands inside the time window.
  %  - backgrounding pressure level is set to 3 dB for numerical stability.
  %  for maskLevId = 1:length(maskLevs),
  %    maskLev = maskLevs(maskLevId); 

    %IO kluge
    maskLevId = 1;
    
    band_dB = 3*ones(1,1);
    onsetlouds( maskLevId, peakid) = max(3, peakpresses(peakid:peakfut));
    onsettimes( peakid ) = round( median(peaktimes(peakid:peakfut)));
  
  %
  % IO comment TYPES simply are the peak pressures see. no background pressures
  %
  
  % "TYPE" means the loudness of the onset components that contribute to
  % the current onset. Thus the spectrum of the background is NOT included.
  onsettypes( peakPressBids(peakid:peakfut), peakid) =...
      peakpresses(peakid:peakfut)'; % ak: ear's eq. L curve no more included
end;


% -Refine raw data for output 
% Remove the added zero from the end of the peaktimes vector
peaktimes = peaktimes(1:end-1);

% Bias the spectral patterns so that smalles dB is 0 in 'types'
onsettypes = onsettypes - min(min(onsettypes));

% Smallest component pressure to zero
peakpresses = peakpresses - min(min(peakpresses));


% overall loudness based on the bankPressure, go to C code to see
% where this is coming from? what does it mean and do we need it?
% 
% ---- OVERALL LOUDNESS that is used in selecting loud enough global onsets
% -Calculate the overall loudness of the signal. Based on signal loudness
%  'loudtimefrac' times per second over time. Band-wise pressure values used.
louds = zeros( 1, floor(loudtimefrac*length(signal)/srate)); % ak:+1 omitted
for id = 1:floor(loudtimefrac*length(signal)/srate), % ak:+1 omitted
  [sones, phons, frqHz, ELdB, sp_loud] =...
      loudnOpt( max( 3, 20*log10(bankPresses(:,id))'-Io),  bands);
  louds(id) = phons;
end;

%
% IO comment whats the value of lboardlen = 0 vs != 0 ?
%
if lboardLen == 0,
  % --- GLOBAL overall loudness level (old way of doing it)
  loudover = sqrt(mean(louds.^2));  % ak: this is better than mean
  disp( ['Signal overall loudness is ', num2str(loudover)]);
else

  % --- LOCAL signal loudness level determination
  %  sqrt( <weighted mean> ( louds.^2)); 
  % where weighting uses raised cosine

  % -Loudness 'surfboard', i.e. weights for running weighted mean
  %  Raised cosine form is derived from ear's dynamic adaptation properties
  %  See Moore "Hearing" pp. 138 for discussion
  lboard = cos(0:pi/(lboardLen*loudtimefrac-1):pi)+1;

  % -Calculate running weighted mean via convolution
  %     (compare : loudover = sqrt(mean(louds.^2)); )
  % remove zero beginning from louds (decimation filter phase delay)
  s=1;while(s<length(louds) & louds(s)<15),s=s+1; end; louds(1:s-1)=louds(s);
  louds = louds.^2;
  louds = conv( louds, lboard) / sum(lboard);
  louds = louds(1:end-length(lboard)+1); % drop the tail
  % handle conv start effect
  s = min( length(louds), ceil(length(lboard)/2.0));
  se = min( length(louds), length(lboard));
  halfSum = sum(lboard(1:s));
  louds(1:s) = louds(s) * sum(lboard) / halfSum;
  louds(s+1:se) =louds(s+1:se) ./(cumsum(lboard(s+1:se))+halfSum) *sum(lboard);
  % take sqrt to arrive at RMS
  louds = sqrt( louds);

  % IO comment do we really need to do this?
  % since we subtract later on we have onsetslouds 
  % we ensure that the least value is max(louds)
  % that is KOSHER!
  %
  % -Manipulate onset data
  %    -make sure that negative values do not appear
  onsetlouds = onsetlouds + max(max(louds));
  
  % what is happening here: voila!
  % onsetlouds = onsetlouds - louds
  %
  for id = 1:length(onsettimes),
    [ignore,tmpid]=min(abs(([0:(length(louds)-1)]*srate/loudtimefrac+1) -...
	onsettimes(id)));
    tmpLval = louds(tmpid);
    onsetlouds(:,id) = onsetlouds(:,id) - tmpLval;
  end;
  
  % -kind of 'global' onset value for exactly correct biasing earlier
  %   that was done just to hinder negats
  loudover = max(louds);
  disp( ['Overall loudness (min,max) :', num2str([min(louds), max(louds)])]);
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ----- SELECT THE GLOBAL ONSETS FOR OUTPUT
%       Prune out too quiet ('loudthreshs') or too-close-to-a-louder onsets
% -onsets must be sorted in ascending time order (C-version of pruneOutOnsetsC)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% -initialize
times = cell( length(maskLevs), length(loudthreshs));
louds = cell( length(maskLevs), length(loudthreshs));

%for maskLevId = 1:length(maskLevs),  % IO comment this == 1, clean up
%  for loudthreshId = 1:length(loudthreshs), % IO comment this == 1, clean up

% IO hack job
maskLevId = 1;
loudthreshId = 1;

loudthresh = loudthreshs(loudthreshId);
    
    % -Adjust the threshold. Threshold 'loudthreshs' is expressed in dB 
    %  relation to the overall level of the signal.
    loudthresh = loudover + loudthresh; 
    % if plotflag,disp(['Loudness threshold is now: ', num2str(loudthresh)]);end;
    
    % IO question, do we need to prune? again?
    % -Select the acceptable onsets 
   % ids = pruneOutOnsets( onsetlouds(maskLevId,:), onsettimes,loudthresh,...
%	mininterval*srate)';
    
    % -Assign to output
    louds{maskLevId,loudthreshId} = onsetlouds(maskLevId, ids);
    times{maskLevId,loudthreshId} = onsettimes(ids);
    types = onsettypes(:,ids);
%  end;
%end;


% ----- Drop out cell form {}, if only one set of onsets is outputted
if max(size(louds))==1,
  louds = louds{1}
  times = times{1};
end;

return;

