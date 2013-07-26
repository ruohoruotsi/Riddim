function tatumVector = getMetricalGrid(onsetVector, sr)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 
% getMetrical grid takes in a series of inter onset intervals (IOIs) 
% and returns the "temporally shortest and perceptually lowest level
% pulse" 
%
% Usage grid  = getMetricalGrid(<vector of onset times>, samplerate)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
plotflag = 0;
remainderVector = [];
maxTatum = sr;
ioi_rez = (5/1000)*sr;
histLength            = ceil(maxTatum/ioi_rez);
gcdRemainderErrLength = ceil(maxTatum/ioi_rez);

errorFunction = mexFillHistograms(onsetVector, sr);

% what is the size of histogram
%tatumIndex = getTatumIndex(histogramVector(7,:), gcdRemainderErrLength, ioi_rez, histLength);
%grid = tatumIndex*ioi_rez/sr;


% tatum calculation ...
% 
alpha = 0.4;
medianVal = 0;
minVal = 1e10;
errorThreshold = .0001;

%errorFunction = errorFunction(12,:);
errorFunction = errorFunction';
minVal = min(errorFunction);
medianVal = median(errorFunction);

% calculate first derivative and find critical points
% calculate second derivative 

% calculate derivatives of everyone
d1 = sign(diff(errorFunction));
d2 = [diff(d1)]';

% just adds zeros to make size d2 == size d1
d2 = [zeros(size(d1,2),1), d2]';  


tatumVector = [];
% get local min peaks/valleys choose biggest
for i = 1:size(d1, 2),
  peaks = find(d2(:,1) == 2);
  temp = find(errorFunction(:,i) == min(errorFunction(peaks,i)));
  tatumVector = [tatumVector; temp];
end

tatumVector = tatumVector*ioi_rez/48000;

%ef = errorFunction(:,10);

%
% end calculate tatum   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  








