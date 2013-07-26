%
% mats to load
%

%
% sexyboy1.mat  

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% artful.mat  
% nice defaults . . . diffPrecision == 4
% w/ get onsets . . .
plotOnsets(y1, 1/6, .85)
plotOnsets(y2, 1/6, .87)
plotOnsets(y3, 1/6, .3)
plotOnsets(y4, 1/6, .88)


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% zingzong1.mat  
plotOnsets(y1, 1/6, .84);
plotOnsets(y2, 1/4, .89)  % this one with diffPrec == 1


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% dreem.mat



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% dreem.mat
clear all;
load sexyboy1;

times = plotOnsets(y1, .1);
getMetricalGrid(times, 48000);

