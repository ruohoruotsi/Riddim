function [times, louds, types, banksigBank, banksigPeaktimeBank, peakPressBids, ...
      peaktimes, peakpresses, peakBgPresses,bankPresses] = extractOnsets(signal, srate,...
    mininterval, loudthreshs, maskLevs, lboardLen, diffPrecision, iothreshhack)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% for diffPrecision
% period used in differentiation 2 is good, 1 is robust
% 4 is good for high threshold
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ONSETS AK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Find the beginnings of discrete acoustic events (onsets of sounds) in
% a time domain acoustic signal. Determine their times and loudnesses.
%
% function [times, louds, types] = onsets( signal, srate,...
%                                      mininterval, loudthreshs)
%  * times   : Indices to the samples in the signal, where an onset of 
%              a sound is detected
%  * louds   : Loudness or the onset in phons (dB).
%  * types   : Rough frequency spectrum of the onsetsetting sounds.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EXTRA OUTPUT - BANDWISE ONSET COMPONENTS
%  * peakPressBids: vector of size 1xN, containing integers
%         between 1 and <number of frequency bands>. Indicates the 
%         band from which the onset component was taken
%  * peaktimes: vector of size 1xN, time of the onset component.
%  * peakpresses: vector of size 1xN, intensity of the onset component
%  * peakBgPresses: vector of size 1xN, intensity of the bandwise
%         signal which is backgrounding this onset component (may be ignored)
%  * bankpresses: do not care.:)
%
%    signal      : time domain sampled signal
%    srate       : sampling rate of 'signal'.
%    mininterval (optional) : minimum accepted interval between two sound 
%                onsets in seconds. Default value is 1/16 (16 onsets per 
%                second). If a mininterval smaller than default value can 
%                be given, it adds robustness for the 'imperfections' in 
%                amplitude envelope onset track, and for AM-modulations.
%    loudthreshs (optional) : threshold for onset approval. Onsets, whose
%                loudness is below signal level (dB) + 'loudthreshs' (dB) are 
%                dropped out. --Default value is -44 dB, in relation to 
%                the signal level).
%                ** NOTE: 'loudthreshs' can be a vector. In this case
%                outputs 'times' and 'louds' will contain vector cells,
%                corresponding to these 'loudthreshs' values. 'types' will
%                contain values for only the last 'loudthreshs' value.
%                Computational cost compared to only one 'loudthreshs' is 
%                very small (= negligible).

plotflag = 1;

% ---- Default parameter values
% Minimum length between note onsets in seconds (=60/tempo).
if ~exist( 'mininterval'),
  mininterval = 1/16;
end;

% Threshold for final selection of loud enough onsets
if ~exist( 'loudthreshs'),
  loudthreshs = -34;
end;

% Level of masking: mask level is <level before onset>-maskLev, and the
% onsetting sound gets level <rise of level> - <level before onset> - maskLev.
if ~exist( 'maskLevs'),
  maskLevs = -56;  % 19 dB below the masker
end;

% lboardLen = 4; % in seconds  ---> 1,2,4,8,16
% Locally adapting loudness window length (seconds, 2-8 s. is good)
lboardLen = 8;

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

% ---- Process signal BAND-BY-BAND. Detect and determine parameters 
%      of the onset components at each band. 
% -Decimation filter. For the sake of efficiency the signal is decimated 
%  during the band-wise calculations. **NOTE: Decimation is done for
%  efficiency. Does NOT have to drop out all frequencies above, say 20 Hz.
Fdec = 100; % Hz. Decimation cutoff frequency
[Bdec, Adec] = cheby1( 6, 1, .5*Fdec/(srate/2)); % 6th order, x dB pass ripple

% - Initializations
peaktimes = [];
peakpresses = [];
peakBgPresses = [];
peakPressBids = [];

% + 1 added back by IO
bankPresses = zeros(Fcount,floor(loudtimefrac*length(signal)/srate) + 1); %ak:+1 omit

% IO for debugging only
banksigBank = [];
banksigPeaktimeBank = [];


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Go through and analyse each band . . . 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for bandid = 1:Fcount,
  if plotflag,
    disp(['analysing band number ', num2str(bandid), ' (',num2str(Fcount),')']);
  end;
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %
  % - ONSET COMPONENTS AT THE BAND: detect and determine their parameters
  [peaktimesB, peakpressesB, peakBgPressesB, bankPressesB] = ...
      mexBandOnsets(Bbank(bandid,:), Abank(bandid,:), signal, srate, ...
      Io, timefrac, loudtimefrac, diffPrecision, Fdec, Bdec, Adec);
  
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %
  % -PRUNE OUT peaks that are at same band and too close to each other.
  %  Otherwise they may override their louder neighbours in the integration.

  % sort, do we need this?
  [peaktimesB,sids] = sort(peaktimesB);
  peakpressesB = peakpressesB(sids);
  peakBgPressesB = peakBgPressesB(sids);

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %  -  Pruned  - 
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %
  % the io jazzy pruning peaktimes first, peakpresses next
  %
  ids = mexPruneOnsets( peaktimesB, peakpressesB,...
      min(peakpressesB), mininterval*srate)';

  % iroro's mexPruneOnsets returns column vectors rather than row
  ids = ids';
  
  % ids = pruneOutOnsets( peakpressesB, peaktimesB,...
  %    min(peakpressesB), mininterval*srate)'
  %ids
  
  peaktimesB = peaktimesB(ids);
  peakpressesB = peakpressesB(ids);
  peakBgPressesB = peakBgPressesB(ids);

  % -STORE- the band-wise data to wait for integration phase
  peaktimes = [peaktimes peaktimesB];
  peakpresses = [peakpresses peakpressesB];
  peakBgPresses = [peakBgPresses peakBgPressesB];
  peakPressBids = [peakPressBids bandid*ones(1,length(peaktimesB))];
  bankPresses( bandid, :) = bankPressesB;

end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ----- COMBINE ONSET COMPONENTS to yield global onsets -----------
%  -Sort collected peaks in time.
%   (In fact this would not be needed at the moment, but sorting is done to
%    make sure that modifications above do not create surprises)
[peaktimes, ids] = sort( peaktimes);
peakPressBids = peakPressBids(ids);
peakpresses = peakpresses(ids);
peakBgPresses = peakBgPresses(ids);


% Without this last peak would not be treated later when bands are combined.
% This is cancelled at a later time.
peaktimes = [peaktimes, 0];

% Put pressures to dB scale
peakpresses =   20*log10( peakpresses) - Io;
peakBgPresses = 20*log10( peakBgPresses) - Io;

% Initializations to zero
onsetlouds = zeros( length(maskLevs), length(peaktimes));
onsettimes = zeros( 1, length(peaktimes));
onsettypes = zeros( Fcount, length(peaktimes));
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
  for maskLevId = 1:length(maskLevs),
    maskLev = maskLevs(maskLevId);
    band_dB = 3*ones(1,Fcount);
    % --simulate masking. Masking level is 'maskLev' below the masker
    %    -old option: limit masking so that it is not ABOVE masker (max(0,..))
    %    -old option: take max(3,..) first separately from p.presses & BgPress
    band_dB( peakPressBids(peakid:peakfut)) = max( 3,...
	peakpresses(peakid:peakfut) -...
	(peakBgPresses(peakid:peakfut) + maskLev));
    % --calculate loudness
    [sones, phons, frqHz, ELdB, sp_loud] = loudnOpt(band_dB, bands);
    onsetlouds( maskLevId, peakid) = phons;
  end;
  
  % TIME is the median of the peaks in 'timefrac' window.
  onsettimes( peakid) = round( median(peaktimes(peakid:peakfut)));
  
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


% ---- OVERALL LOUDNESS that is used in selecting loud enough global onsets
% -Calculate the overall loudness of the signal. Based on signal loudness
%  'loudtimefrac' times per second over time. Band-wise pressure values used.
louds = zeros( 1, floor(loudtimefrac*length(signal)/srate)); % ak:+1 omitted
for id = 1:floor(loudtimefrac*length(signal)/srate), % ak:+1 omitted
  [sones, phons, frqHz, ELdB, sp_loud] =...
      loudnOpt( max( 3, 20*log10(bankPresses(:,id))'-Io),  bands);
  louds(id) = phons;
end;

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

  % -Manipulate onset data
  %    -make sure that negative values do not appear
  onsetlouds = onsetlouds + max(max(louds));
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
[onsettimes,sids] = sort(onsettimes);
onsetlouds = onsetlouds(:,sids);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% IO hard thresholding hack!!
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

y = onsetlouds;
y = (y .* [y >= iothreshhack*max(y)]) + ...
    (y .* [y <= -1*iothreshhack*max(y)]);

onsetlouds = y;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% -initialize
times = cell( length(maskLevs), length(loudthreshs));
louds = cell( length(maskLevs), length(loudthreshs));
for maskLevId = 1:length(maskLevs),
  for loudthreshId = 1:length(loudthreshs),
    loudthresh = loudthreshs(loudthreshId);
    % -Adjust the threshold. Threshold 'loudthreshs' is expressed in dB 
    %  relation to the overall level of the signal.
    loudthresh = loudover + loudthresh; 
    if plotflag,disp(['Loudness threshold is now: ', num2str(loudthresh)]);end;
    % -Select the acceptable onsets
    ids= pruneOutOnsets( onsetlouds(maskLevId,:), onsettimes,loudthresh,...
			  mininterval*srate)';
    % -Assign to output
    louds{maskLevId,loudthreshId} = onsetlouds(maskLevId, ids);
    times{maskLevId,loudthreshId} = onsettimes(ids);
    types = onsettypes(:,ids);
  end;
end;

% ----- Drop out cell form {}, if only one set of onsets is outputted
if max(size(louds))==1,
  louds = louds{1};
  times = times{1};
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ----- plottings, if desired
%
if 0 & plotflag,
  figure(1); clf; hold on;
  for id = times,
    x = id;
    plot( [x x], [1,21], 'k');
  end
  plot( onsettimes(1:end-1), onsetlouds(1:end-1));
end

return;



