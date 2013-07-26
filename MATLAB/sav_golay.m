function T2 = sav_golay(T,l,forder,dorder)

% function T2 = sav_golay(T,l,forder,dorder)
%

% Polynomial filtering method of Savitsky and Golay 
% Y = vector of signals to be filtered
%     *** the derivative is calculated for each ROW  ** 
% l = filter length
% forder = filter order (2 = quadratic filter, 4= quartic)
% dorder = derivative order (0 = smoothing, 1 = first derivative, etc.) 
%


[m,n] = size(T);
dorder = dorder + 1;

% *** check inputs ***
if  rem(l,2)-1 ~= 0
  error('filter length is not an odd integer')
elseif (forder) < (dorder)
  error('the derivative order is too large')  
end 


% *** calculate filter coefficients ***
lc = (l-1)/2;                    %index
X = [-lc:lc]'*ones(1,forder+1);  
p = ones(l,1)*[0:forder];        %polynomial terms
X = X.^p;                        % polynomial coefficients
F = pinv(X);                     % invert


% *** filter via convolution and take care of the end points ***
T2 = conv(T,F(dorder,:));
T2 = T2(lc+1:length(T2)-lc);

%
% that's all folks . . .