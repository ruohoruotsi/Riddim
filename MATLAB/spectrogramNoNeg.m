function [array] = spectrogram(wave,segsize,nlap,ntrans);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% function array = spectrogram(wave, segsize, nlap, ntrans);
%
% segsize is the windowsize
%
% nlap is number of hamming windows overlapping a point;
%      it actually is the how many samples we skoot over for the next window
%      it also gives us our time resolution in terms of windowsize
%      segsize/nlap is actually the effective window size
%
% ntrans is factor by which transform is bigger than segment;
%        is actually the size of the frequency axis in terms of the window
%        size, so ntrans == 1 = frequency axis == window size, 2* == twice
%        as many bins
% not anymore ==> returns a spectrogram 'array' with fourth root of power,
% filter smoothed and formatted for display
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% some kind of filtering going on here . . . keep it around
wave = filter([1 -0.95],[1],wave);

s = length(wave);
nsegs = floor(s/(segsize/nlap))- nlap + 1; % Number of total windows . . .

%array = zeros(ntrans*segsize, nsegs); % number of windows * window length
array = zeros(.5*ntrans*segsize, nsegs); % number of windows * window length

% hamming window of size segsize
window = hamming(segsize);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% windowing operation
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i = 1:nsegs
  seg = zeros(ntrans*segsize,1); % leave half full of zeroes
  seg(1:segsize) = ...
      window.*wave(((i-1)*segsize/nlap+1):((i+nlap-1)*segsize/nlap));
  seg = abs(fft(seg));
  
  % lop off the negative frequencies . . .
  array(:,i) = seg(((.5*ntrans*segsize)+1):(ntrans*segsize)); 
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
raw = array;
array = array .* array;			
array = array .^ .5;  % get the square of the magnitude

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%for i=1:nsegs    % smooth the spectral slices
%  array(:,i) = filter([.2 1 .2],[1],array(:,i));
%end
%
%for i=1:.5*ntrans*segsize    % smooth the channels
%  array(i,:) = filter([.2 1 .2],[1],array(i,:));
%end

