function [times, louds]  = plotOnsets(y, persec, thresholdValue)
%%%%%%%%%%%%%%%%%%%%%%%%%%%

% iroro test driver for getOnsets
%%%%%%%%%%%%%%%%%%%%%%%%%%%
% arguments . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%

sr = 48000;  

% using klap's loudness code
%[times, louds, types] = extractOnsets(y, sr, persec, -26, -56, 0, 4, iothreshhack);

% using all io stuff
[times, louds] = getOnsets(y, sr, persec, -26, -56, 0, 2, thresholdValue);

% using klap's everything
%[times, louds, types] = onsetsAkm(y, sr, persec, -26, -56, 0);

% set up grids . . .
plot(y);
set(gca, 'XTick', times);
set(gca, 'XColor', [1 0 0]);
set(gca, 'XTickLabel', []);
grid on;
set(gca, 'YGrid', 'off');
set(gca, 'Color', [0 0 0]);
%louds