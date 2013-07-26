function [sones, phons,bandCenters,bandEL,sp_loud] = loudnOpt(band_dB, bands),
% LOUDN OPT
%
% **** THIS IS OLD STUFF. USE loudnOptC.c (verified mex-version)
%
%
% Calculate the LOUDNESS (in sones and in phons) of a signal
% from a NUMBER OF FILTERBANK OUTPUTS. Also excitation levels and specific
% loudnesses (loudnesses at each critical band) are given.
%
% Input bands should be approximately logarithmically distributed. For example
% pink noise with all band levels of 67 dB: band_dB=67*ones(1,bandCount).
% FREE FIELD from field to eardrum is assumed.
%
% NOTE: This version is optimized from my 'loudn.m', which in turn was
% optimized from Nokia's 'newloud.m'. Several simplifications have took place.
%
% [sones, phons, frqHz, ELdB, sp_loud] = loudn(band_dB, bands)
%  * band_dB  : vector of 26 elements. Sound pressures at the outputs of the
%              filterbank, whose frequency boundaries are given in 'bands'.
%  * bands   : Matrix or size (2,<number of bands>). The first column 
%              contains lower boundary of each frequency band, the second 
%              column the higher boundaries. Use Hz units. Logarithmic 
%              distribution of bands should cover the range 50-15000 Hz.
%    sones   : loudness of the signal. Sone is unit of loudness level 
%              of a sound (subjective loudness). LINEAR SCALE (a sound of
%              2 sones is twice as loud, 0.5 sones is half as loud).
%              One sone is defined as the loudness experienced by a person 
%              listening to a tone at the 40-phon loudness level. 
%              Here 'sones' are monaural loudness.  To get binaural loudness 
%              multiply by 2.
%    phons   : unit of loudness level. DB SCALE. Phon is defined as the 
%              sound pressure level in dB of a 1000 Hz tone with 
%              the same subjective loudness. Here 'phons' are monaural values.
%              To get binaural phons run loudnso2phons(sones*2).
% (((   frqHz   : frequency points, where 'ELdB' and 'sp_loud' are calculated.
%    ELdB   : Excitation level at frequency points of 'frqHz'.
%    sp_loud : specific loudnesses at frequency points 'frqHz'.  )))
%    
% Optimized from: loudn.m by Anssi Klapuri, which in turn was
% Optimized from: version 4.0 of Original fortran code by Moore et. al.
%     Author(s): PeKu, Date: 14.1.1998
%     Copyright (c) 1998 by Nokia Research Center, Audio Coding.
% Reference:
%     "A Model for the prediction of Thresholds, Loudness, and 
%      Partial Loudness". Moore et. al. J of AES #4, 1997, pages 224-240
% Programmed 4.5.99. Anssi Klapuri, klap@cs.tut.fi.

c1=24.673;
c2=4.368;
c3=2302.6/(c1*c2);
const=0.046871;

% Center frequencies for each band
bandCenters = mean(bands')';
bandCount = size(bands,1);

% Fixed filter for transfer from free field to eardrum and through 
% middle ear. Free field is assumed (not diffuse, for example).
if length(bandCenters)==8 & all(bandCenters==[97.5  225*2.^(0:5) 14300]'),
  SAM=[-12.680853 -7.076319 -2.444277 0.00 2.850 9.10 -6.182353 -15.424800];
else,
  SAM=shawAndMid( bandCenters)';
end;
compdB = band_dB + SAM;

% The same in linear scale
complin = 10.^(compdB/20);

% --- Go through the bands and sum up the loudness
sones = 0; 
for bandId = 1:bandCount,
  band = bands(bandId,:);
  % -band width in ERB
  Ewidth = c3*log10(c2*band(2)/1000+1) - c3*log10(c2*band(1)/1000+1);
  % -Excitation level for this band.
  %     -scaler is calculated by a function I fitted to sum(LOUDELdBspread')
  %      using [a,e]=fitexpL2(erbValues,sum(LOUDELdBspread'))
  compLinScaler = 2.674777*loudnHz2erb( bandCenters( bandId)).^(-0.5655041);
  Esig = compLinScaler * complin(bandId);
  sp_loud(bandId) = 0;
  if compdB(bandId) > 3.6-30,
    if Esig <= 1e10, % The typical case in the normal hearing range
      sp_loud(bandId) = Ewidth * const * ((Esig*1+4.5817)^.2 - 4.5817^.2);
    else, % Very loud
      sp_loud(bandId) = Ewidth * const * sqrt(Esig/1.04e6);
    end
    if Esig < 2.2909, % below hearing threshold sp_loud must not go 0
      sp_loud(bandId) = sp_loud(bandId) * ((2*Esig)/(Esig + 2.2909))^1.5;
    end;
    sones = sones + sp_loud(bandId);
  end
end

%sones = sones / 2;
phons =  loudnso2phons(sones);  % Monaural
bandEL = complin;
return;


% ----------------------------------------------------------------------

function outSAM = shawAndMid( freqHz)
% SHAW AND MID
%
% Fixed frequency response for transfer from free field to eardrum,
% for given set of frequencies.
%
% output = shawAndMid( freqHz)
%   * freqHz : frequency points, at which the response is given
%     output : frequency response (in dB scale) for the transfer from
%              free field to eardrum. The data is relative to  0.0 dB 
%              at 1000 Hz.
%
% Programmed 4.5.99. Anssi Klapuri, klap@cs.tut.fi.
% Original data typed in by Nokia's coder slaves.

% -frequency points at which the response was sampled from 
%  (ISO std Table 1 - 4.2 dB --> ISO 226? -ak)
samHz=[ 0.,    20.,    25.,   31.5,    40.,    50.,    63.,    80.,...
   100.,   125.,   160.,   200.,   250.,   315.,   400.,   500.,...
   630.,   750.,   800.,  1000.,  1250.,  1500.,  1600.,  2000.,...
  2500.,  3000.,  3150.,  4000.,  5000.,  6000.,  6300.,  8000.,...
  9000., 10000., 11200., 12500., 14000., 15000., 16000., 20000.];

% --- Fixed filter for transfer from FREE FIELD to eardrum.
%     Vector "shaw_cor - mid_ear" taken from 'newloud'.
shawAndMidVals =...
    [-50,   -39.15,   -31.4,   -25.4,   -20.9,   -18.0,   -16.1,   -14.2,...
      -12.5, -11.03,   -9.41,   -7.92,   -6.3,    -4.7,   -3.1,    -2.0,...
      -0.5,     0,      0,       0,      0.5,     1.5,    2.0,    3.5,...
      6.0,    8.0,      8.5,    8.5,    5.0,     -0.5,   -2.0,   -9.5,...
      -11.5,  -11.5,  -10.0,   -9.0,    -14,    -19.3,  -15.3,    -17.5];

% --- Simply closest match between the values in 'samHz' and 'freqHz'
%     'freqHz' is assumed to be in ascending order.
outSAM = zeros(size(freqHz));
oldId=1;
for outId = 1:length(freqHz),
  [ignore, match] = min( abs( samHz(oldId:end) - freqHz(outId)));
  outSAM(outId) = shawAndMidVals(oldId+match-1);
  oldId = oldId+match-1;
end
return;
