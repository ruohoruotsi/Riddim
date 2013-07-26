function [time_series, array] = isatest(wave,segsize,nlap,ntrans);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% try: [time_series, array] = isatest(x, floor(length(x)/4), 1,1);
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% function array = spectrogram(wave,segsize,nlap,ntrans);
% defaults spectrogram(wave,128,8,4)
% nlap is number of hamming windows overlapping a point;
% ntrans is factor by which transform is bigger than segment;
% returns a spectrogram 'array' with fourth root of power,
% filter smoothed and formatted for display
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% some kind of filtering going on here . . .
wave = filter([1 -0.95],[1],wave);

s = length(wave);
nsegs = floor(s/(segsize/nlap))-nlap+1; % Number of total windows . . .
array = zeros(.5*ntrans*segsize,nsegs); % number of windows * window length

% hamming window of size segsize
%window = 0.54-0.46*cos(2*pi/(segsize+1)*(1:segsize)'); 
window = hamming(segsize);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% windowing operation
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i = 1:nsegs
 seg = zeros(ntrans*segsize,1); % leave half full of zeroes
 seg(1:segsize) = ...
     window.*wave(((i-1)*segsize/nlap+1):((i+nlap-1)*segsize/nlap));
 seg = abs(fft(seg));
 array(:,i) = seg(((.5*ntrans*segsize)+1):(ntrans*segsize)); % grab upper half
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% back into power domain for smoothing
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

raw = array;
array = array .* array;			

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i=1:nsegs    % smooth the spectral slices
  array(:,i) = filter([.2 1 .2],[1],array(:,i));
end

for i=1:ntrans/2*segsize    % smooth the channels
  array(i,:) = filter([.2 1 .2],[1],array(i,:));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% compress with square root of amplitude (fourth root of power) 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

off = 0.0001*max(max(array));      	% low end stabilization offset,
array = (off+array).^0.25-off^0.25; 	% better than a threshold hack!
array = 255/max(max(array))*array;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% do fastica and get back signals to flop
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
raw = raw';
ica = fastica(raw);

[ica_rows, ica_col] = size(ica);

fprintf('doing IFFT transformation . . .just hold that thought!!\n');

for i = 1:ica_rows
  time_series(i,:) = ifft(ica(i,:));
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% truncate the complex values . . . don't need them, miniscule @ best
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
time_series = real(time_series);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% how about some frikken plotting???
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


