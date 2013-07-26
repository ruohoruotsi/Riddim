numiterations = 50;
sr = 48000;
baseioi = sr/2;
riddimPattern = [];



louds = [];
infilename = 'in-hat.wav';
outfilename = 'changingTatum';
threshold = .05;


% static file to read from for sound for now with matching 
% static sample rate == 48k
y = wavread(infilename);
ylen = length(y);

% the riddimPattern will be in sample values
accum = 0;
for i = 1:numiterations,
  diff = (baseioi - ylen) * (i/numiterations);
  accum = accum + ylen + floor(diff);
  riddimPattern = [riddimPattern, accum];
  louds = [louds, .7];
end


% create a vector the size of riddimPattern[length(riddimPattern)] + ylen
out = zeros(1, riddimPattern (length(riddimPattern)) + ylen);

maxlouds = max(louds);

% i increments the samples
% j increments the riddimPattern

i = 1;
for j = 1:length(riddimPattern),
  i = riddimPattern(j);
  out(i:i + ylen - 1) = y*((louds(j) + threshold)/maxlouds);  % y scaled by max louds 
end

% write/play back the pattern
%sound(out, 48000);
wavwrite(out, 48000, cat(2, outfilename, '.wav'));