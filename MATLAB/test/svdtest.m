%
% testing out svd to see how to get our damn basis vectors
% 

clear all;
x = wavread('wavs/test.wav');
x = x(1:22050);

nsegs = 40;
%[array, raw] = spectrogram(x, floor(length(x)/nsegs), 1,1);'
[array] = spectrogram(x, floor(length(x)/nsegs), 1,1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% watch out that assumptions and facilitators don't
% bite your butt with bad/ugly data . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

figure;
imagesc(array)

% take the transpose to make the temporal dimension first
array = array';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% svd those mudda's
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% just get the 'V' vector from the [u,s,V] = svd() 
%

clear x;
clear raw;

for i=1:nsegs    
  [null, null, v1(:,:,i)] = svd(array(i,:));
end;  

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% save only the first n=8 components of the SVD as the
% independent components
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i=1:nsegs
  v2(:,:,i) = v1(:,1:8,i);
end;
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% pass each mudda to ICA for extraction
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% do an abs to get rid of the negative business
%
for i=1:nsegs
  %  icasig(:,:,i) = abs(fastica(v2(:,:,i)'));  
  icasig(:,:,i) = fastica(v2(:,:,i)');  
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% reconstruct some spectrograms . . biatch!!
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
for i=1:nsegs
  s(:,i,1) = icasig(1,:,i);  % necessary for each subspace spectrogram
  s(:,i,2) = icasig(2,:,i);  
  s(:,i,3) = icasig(3,:,i);  
  s(:,i,4) = icasig(4,:,i);  
  s(:,i,5) = icasig(5,:,i);  
  s(:,i,6) = icasig(6,:,i);  
  s(:,i,7) = icasig(7,:,i);  
  s(:,i,8) = icasig(8,:,i);  
end;


figure;  
imagesc(s(:,:,1))

figure;  
imagesc(s(:,:,2))

figure;  
imagesc(s(:,:,3))

figure;  
imagesc(s(:,:,4))

figure;  
imagesc(s(:,:,5))

figure;  
imagesc(s(:,:,6))

figure;  
imagesc(s(:,:,7))

figure;  
imagesc(s(:,:,8))