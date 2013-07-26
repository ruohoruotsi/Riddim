%function S = stft(x, w, sampling_freq, freq_step, sample_step) 

%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
w = hanning(256);
x = wavread('greenstart.wav');

f = 128;
w = 128;
h = 32;


len_w = length(w) - 1; 
len_x = length(x); 

scrsz = get(0,'ScreenSize'); 
H1 = figure('Position',[(3*scrsz(3))/4 1  scrsz(3)/4 scrsz(4)/4]); 

tv = 0:len_x-1; 
tv = (1/sampling_freq).*tv; 
Sx = []; 
f1v = []; 

for f1 = 0:freq_step:(sampling_freq/2) 
   f1v = [f1v f1]; 
   figure(H1) 
   title(f1) 
   exp_fun = exp(-i*2*pi*f1.*tv); 
   S1v = []; 
   n1 = 1; 
   while n1 < (len_x - sample_step - len_w - 1), 
      S1 = (1/sampling_freq)*sum(x(n1:n1+len_w).*w.*exp_fun(n1:n1+len_w)); 
      S1v = [S1v S1]; 
      n1 = n1 + sample_step; 
   end 
   Sx = [Sx;S1v]; 
end 


close(H1); 
figure 
subplot(211), plot(tv, x); 
grid, title('signal'); 
xlabel('time (sec)'); ylabel('amplitude'); 
axis([0 max(tv) 1.5*min(x) 1.5*max(x)]); 

Ss = size(Sx); Ss = Ss(1,2); 
t1v = tv(1:sample_step:length(x)-sample_step-len_w-1); t1v = t1v(1:Ss); 
subplot(212), imagesc(t1v, f1v, abs(Sx).^2); axis xy; 
title('STFT'), 
xlabel('time (sec)'); ylabel('frequency (Hz)'); 
S = Sx; 

