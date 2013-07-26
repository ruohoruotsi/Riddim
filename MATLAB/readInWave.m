function [y]= readInWave(filename)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% READINWAVE(filename) takes a string variable filename and 
% returns a vector containing the databuffer 'buffer', the sampling
% frequency 'sf' and the number of bits per sample 'bitDepth'.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[buffer, sf, bitDepth] = wavread(filename);

y = length(buffer);
