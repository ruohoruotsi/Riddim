load zingzong1;
[times, louds] = plotOnsets(y4, 1/16, .23);
saveRiddimPatternMIDI(times, louds, 'crap');