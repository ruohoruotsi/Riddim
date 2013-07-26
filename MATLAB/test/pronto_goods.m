function [buffer] = pronto_goods(filename)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% iroro orife october 2000 
% prototype for rhythm analysis system
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[buffer, sf, bitDepth] = wavread(filename);

[time, Energy, maxi, stride] = energy(buffer, sf);
[time, HFC, maxi, stride] = hfc(buffer, sf);


%% THE DETECTION FUNCTION

column      = size(time, 2);
vec_detect  = [HFC(1)/Energy(1)];
vec_squared = HFC .* HFC;


vec_multipl = HFC(1:column-1) .* Energy(2:column);
vec_detect  = [vec_detect vec_squared(2:column) ./ vec_multipl];


buffer_time = [0:1/(sf):(1/sf)* length(buffer) - (1/sf)];

figure;
subplot(211), plot(buffer_time, buffer);
title('original');
grid;

subplot(212), plot(time, HFC);
title('High Frequency Content');
grid;

figure;
subplot(211), plot(time, Energy);
title('Energy');
grid;

subplot(212), plot(time, HFC);
title('High Frequency Content');
grid;
