function erb = loudnHz2erb( freqHz)
% LOUDN HZ 2 ERB
%
% Frequency (Hz) to ERB conversion.
%
%  erb = loudnHz2erb( freqHz)
%
% Programmed 22.2.1999. Anssi Klapuri, klap@cs.tut.fi.

c1=24.673; 
c2=4.368; 
c3=2302.6/(c1*c2);

erb = c3*log10(c2*freqHz/1000+1);


