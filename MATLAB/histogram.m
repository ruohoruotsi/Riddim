x1 = wavread('x1.wav');
x2 = wavread('x2.wav');
x1 = x1';
x2 = x2';

x2 = x2(1:length(x1));