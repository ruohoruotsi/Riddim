function [] = rsound( s, sr)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% play a sound @ a specified sampling rate, but first
% remove the mean and scale/normalize
%
%

if nargin < 2
  % sr = 8000;
  sr = 16000;
end

% remove mean and scale sound 
s = s - mean( s);
m = 1.1 * max( abs( s(:)));

% play me!
sound(s/m, sr);

%wavwrite(s/m, sr, 'gkc_out1.wav');