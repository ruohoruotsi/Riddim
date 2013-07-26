function [times, louds] = plotOnsets(y, persec, iothreshhack)
%%%%%%%%%%%%%%%%%%%%%%%%%%%

% iroro test driver for getOnsets
%%%%%%%%%%%%%%%%%%%%%%%%%%%
% arguments . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%

sr = 48000;  

% using klap's loudness code
%[times, louds, types] = extractOnsets(y, sr, persec, -26, -56, 0, 4, iothreshhack);

% using all io stuff
[times, louds, types] = getOnsets(y, sr, persec, -26, -56, 0, 2, iothreshhack);

% using klap's everything
%[times, louds, types] = onsetsAkm(y, sr, persec, -26, -56, 0);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% get Inter-Onset Interval (IOI) lengths
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i = 1:length(times) - 1,
  IOI(i) = times(i+1) - times(i);
  IOI_in_seconds(i) = IOI(i)*(1/sr);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% create an onset vector with spikes where there are onsets . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
onset_vector = zeros(length(y), 1);

%for j = 1:length(times),
%  onset_vector(times(j)) = 2;
%end


% hack . .
for j = 1:length(times),
  if times(j) ~= 1 & louds(j) > 0
    onset_vector(times(j)) = 2;
  end
end


% length - 1, transposed, can you say "hack job"?
t = (0:length(y)-1)';

y = y+1;

% plot onsets and other wave on the same axes
plot(t,y, t, onset_vector, 'r'); 



