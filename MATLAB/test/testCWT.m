function [] = testCWT(y1)

y1 = decimate(y1, 6);
ccfs = cwt(y1, 1:32, 'sym1', 'plot');
colormap(hot);

figure;
plot(y1);