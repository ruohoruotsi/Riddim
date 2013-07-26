%%%%%%%%%%%%%%%%%%%%%%%%%%%
% iroro test driver for getOnsets
%%%%%%%%%%%%%%%%%%%%%%%%%%%
% arguments . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% why IO?
% because data is currently coming out of gkc @ 16k
% interp permits interpolation up in integral factors
% so sr = 16k*3 = 48
%

sr = 48000;  
y = gkc1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% onset times are returned in terms of sample indices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%
% having only one band is okay, but not adequate to pick up riddims
%
%[times, louds, types] = getOnsets(y, sr, 1/16, -26, -56, 0, 4);
%[times, louds, types] = getOnsetsBAK(y, sr, 1/16, -26, -56, 0, 2);


%
%[times, louds, types] = onsetsAKm(y, sr, 1/8, -26, -56, 0);
% benchmark
%[times, louds, types] = extractOnsets(y, sr, 1/16, -26, -56, 0, 4);
[times, louds, types] = extractOnsets(y, sr, 1/4, -26, -56, 0, 4);


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


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% debugging code to see exactly where on the slope the onset is being 
% put and how that works in regard to a Recycle(tm) style onset marker
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% clear onset_vector;
% onset_vector = zeros(length(banksigBank),1);
%
% for j = 1:length(banksigPeaktimeBank),
%  onset_vector(banksigPeaktimeBank(j)) = .15;
% end


% plot the envelopes by themselves . . .
%size(banksigBank)
%size(banksigPeaktimeBank)
%t = (0:length(banksigBank(1,:))-1)';

% plot(t, banksigBank(6,:), t, onset_vector, 'r'); 

% t, banksigBank(2,:), ...
%   t, banksigBank(3,:), t, banksigBank(4,:), ...
%   t, banksigBank(5,:), t, banksigBank(6,:), ...
%   t, banksigBank(7,:), t, banksigBank(8,:), ...

%tg = tempogram(IOI_in_seconds);

%figure;
%plot_tempogram(tg);

