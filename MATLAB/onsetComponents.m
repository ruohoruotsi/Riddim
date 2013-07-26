%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ------------------------------------------------------------------------
%
function [peaktimesB, peakpressesB, peakBgPressesB, bankPressesB, banksig, banksigPeaktimes] =...
    onsetComponents( B, A, signal, srate,...
    Io, timefrac, loudtimefrac,....
    Fdec, Bdec, Adec)
% ONSET COMPONENTS
%
% Detect and determine parameters of the onset components at frequency band.
% Also the overall sound pressure level at this band is calculated at 
% discrete points.
%
% Programmed 1.9.98, 29.6.99. Anssi Klapuri, klap@cs.tut.fi.

plotflag = 0;

% ----- Initializations
% Decimation step in samples. (Decimation is just for the sake of efficiency)
decstep = round( (srate/2)/Fdec);
timefracdec = round(srate*timefrac/decstep);

% Period used in discrete differentiations: (timefrac/2)/precision. Bigger prec
% makes the algorithm unstable for onset imperfections, but gives better onset
% values. 2 is good, 1 is extremely robust, 4 is good for high 'thresh'.
precision = 2; % multiply by 2 to compare to older rhytonsets3rd*

% Length that is used for discrete-time differentiation of decimated signal
%    D(t) = decSig(t+difflen) - decSig(t)
difflen = round( timefrac*srate/(precision*decstep));


% Surfboard for smoothingh the signal envelope by convolving with this
% second half of X ms long hanning window (auditory grounds for use).
% -Multiplication by 2 to assure that sboard really supresses the frequency
%    'timefrac'. This must be done, because the tail of the rais.cos is low
sboardlen = round(2*(timefrac*srate)/decstep);
sboard = .5*(1 - cos(2*pi*(sboardlen:2*sboardlen)/(2*sboardlen)));

% Gaussian function for the sake of making tradeoff between amplitude
% envelope slope and cumulative difference in determining the LOUDNESS
% of the onset component.
% Length is 2*timefrac samples. 72 % of mass of slopegauss is inside timefrac.
slopegauss = gaussmfAK( 0:1/(timefracdec/1):1, [.25 .5]);

%slopegauss = [0 0 0 1 0 0 0];

% --------- Process the signal now at this band
% --- Initializations
peaktimesB = [];
peakpressesB = [];
peakBgPressesB = [];
peakPressBids = [];
bankPressesB=zeros(1,floor(loudtimefrac*length(signal)/srate)); % ak:+1 omitted

% IO debugging only
banksigPeaktimes = [];

% ------- Calculation of banksig
% Filter with the bank   *SLOW
banksig = filter( B, A, signal);

% full wave Rectify
% banksig = abs( banksig);

y = banksig;
banksig = (y .* [y >= 0]);

% Decimate   *SLOW
banksig = filter( Bdec, Adec, banksig);
banksig = banksig(1:decstep:end);



% Remove numeric precision problems 
% ('banksig' may contain small negative values like -2e-4)
banksig = max( 1e-16, banksig);
% Smooth with the raised cosine surfboard. Auditory grounds for use.
%    *FIXME: why not divide with sum(sboard) to retain the signal level
banksig = conv( banksig, 2*sboard/sboardlen);
banksig = banksig(1:length(banksig)-sboardlen+1);
% ----- Collect the pressure values from the band 'loudtimefrac' per second
%    (to calculate overall loudness later)
for id = 1:floor(loudtimefrac*length(signal)/srate), % ak:+1 omitted
  bankPressesB( 1, id) = ...
      max( 1e-16, banksig( round((id-1)*srate/decstep/loudtimefrac)+1));
end;

% ---------- Detecting onset components and determining their times
%            and loudnesses  
% ---- Candidate: find boundaries of the next positive period in diff(banksig)
%    ***SLOW (I suppose)

% why is this here??
% how big will our arrays be??
%(length(banksig)-difflen)/difflen

id = 1;
while id < (length(banksig)-difflen),
  % Scan to next positive period
  while( id < (length(banksig)-difflen) &...
	(banksig(id+difflen) - banksig(id) <= 0) ),
    id = id+1;
  end
  bott = id;
  % Scan to the end of the positive period
  while( id < (length(banksig)-difflen) &...
	(banksig(id+difflen) - banksig(id) > 0) ),
    id = id+1;
  end
  up = id;
  if(banksig(id+difflen) - banksig(id) <= 0), up = up-1; end;
  if bott <= up,
    % --- Differentiate:
    %  -Absolute change (differentiate)
    %   Interrel. of indeces: <diffsig: 1...> <banksig: 1+floor(difflen/2)..>
    diffsig = (banksig(difflen+[bott:up]) - banksig(bott:up));
    % ---- Determine the loudness of the onset component
    % Find the maximum point in diffsig period
    [ignore, maxid] = max(diffsig);
    % Sum diff signal around the maximum slope, emphasize gaussially
    ldeviat = min( floor(timefracdec/2), maxid-1);
    rdeviat = min( floor(timefracdec/2), length(diffsig)-maxid);
    % Rise (sound pressure of the added amount of sound)
    riseids = [-ldeviat:rdeviat];
    peakpressLin = sum( slopegauss(round(timefracdec/2)+1+riseids).*...
	diffsig(maxid+riseids)); % do NOT divide with sum(slopegauss) !
    % Backgrounding signal level. Two possibilities
    
    if 1,
      % -calc before the onsetting sound begins
      % peakBgLin = sum(slopegauss(round(timefracdec/2)+1-ldeviat:...
      %  round(timefracdec/2)+1+rdeviat)) * banksig(bott);
      peakBgLin = banksig(bott); % do not multiply with sum(slopegauss)
    else
      %   -calc in the place as the rise (bad:onsetting sound is there already)
      peakBgLin = sum(slopegauss(round(timefracdec/2)+1-ldeviat:...
	  round(timefracdec/2)+1+rdeviat).*...
	  banksig(maxid-ldeviat:maxid+rdeviat));
    end;
    
    % Convert to dB scale and store to vector
    %peakpressesB =   [peakpressesB,   max( 3, 20*log10( peakpressLin) - Io)];
    %peakBgPressesB = [peakBgPressesB, max( 3, 20*log10( peakBgLin) - Io)];
    % Return to caller in LINEAR scale
    peakpressesB =   [peakpressesB,   peakpressLin];
    peakBgPressesB = [peakBgPressesB, peakBgLin];
    
    
    % ---- Time of the onset component: maximum reldiff value in the period
    %    [ignore, maxrelid] = max( diffsig ./...
    %	banksig( floor(difflen/2)+[bott:up]) );
    
    
    % IO insite: bott:up will give you the exact peak of the whole thingy
    % which isn't really what you want which is just before the thingy
    % where you would put the slider marker to cut in Recycle(tm)
    [ignore, maxrelid] = max( diffsig ./...
	banksig([bott:up]) );
    
    maxrelid = bott + round(mean(maxrelid)) - 1;
    
    % Convert time values from 'maxrelid' to actual signal indeces:
    % Middle of the winning relative slope is taken as rel diff maximum.
    % Thus the event causing it is assumed to be in 1/4 point of the slope.
    % -ak 3.5.99: Changed to be in the middle point, anyway. Better.
    banksigPeaktimes = [banksigPeaktimes, maxrelid];
    peaktimesB = [peaktimesB, decstep*(maxrelid-1)+1+round(decstep/2)];
    
    % IO -hack
    % peaktimesB = [peaktimesB, decstep*(maxrelid-1)+1+round(decstep*3)];
    
  end
end



