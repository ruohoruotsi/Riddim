%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% lets get us some basis vectors . . .
% 

clear all;
x = wavread('wavs/mix.wav');
x = x(1:22050);

nsegs = 80;
rho = 8;  % the number of basis vectors per Z{j}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% toggle between including or not the negative frequencies
%
%[array] = spectrogram(x, floor(length(x)/nsegs), 1,1);
%[array] = spectrogram(x, 128, 2,2)
[array] = spectrogramNoNeg(x, 128, 2,2);
%[array] = spectrogramNoNeg(x, floor(length(x)/nsegs), 1,1);

figure;
imagesc(array)


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% take the transpose to make the temporal dimension first
% this way for X = [U,S,V] X=mxn U=mxm S=nxm & V=nxn
% since the basis vectors are in V, we transpose X
array = array';
[null, d , v] = svd(array);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% pick off the rho basis vectors from p, row instead of 
% column vectors? 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% pick off the first rho
vv = v(1:rho,:);