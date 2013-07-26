clear all;
load ploddingThruSegmentation.mat 

[peaktimesB1, peakpressesB1, peakBgPressesB1, bankPressesB1] = bandOnsets(Bbank(5,:), Abank(5,:), signal, srate, Io, timefrac,loudtimefrac, Fdec, Bdec, Adec);


