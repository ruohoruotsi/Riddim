function ent = entropyMeasure(in1, in2)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ENTROPYMEASURE the measurement of the entropy between two signals
% is minimal when the signals are independent, and maximal when 
% they are correlated 
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% clear;
% rnd  = 16;
vectorLength = length(in1);
rnd = floor(sqrt(vectorLength));
%rnd = 16;

%in1      = rand(1,256);
%in2      = in1 + 0.1*rand(1,256);

%
% what the hell is going on here ?
% everything is getting scaled b/w 0 & 16=rnd
%
%in1      = round( in1 * (255/rnd)/max(in1) );
%in2      = round( in2 * (255/rnd)/max(in2) );

in1 = round( in1 * (vectorLength/rnd)/max(in1));
in2 = round( in2 * (vectorLength/rnd)/max(in2));

dim     = size( in1, 2);
joint   = zeros( vectorLength/rnd + 1 );

for i = 1 : dim
        joint( in1(i)+1, in2(i)+1 ) = joint( in1(i)+1, in2(i)+1 ) + 1;
end
jointi  = joint / sum(sum(joint));  % double sum for 2-d vectors


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% COMPUTE ENTROPY
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
x1      = in1;
[N1,X1] = hist( x1, rnd+1 );
N1      = N1 / sum(N1);
x2      = in2;
[N2,X2] = hist( x2, rnd+1 );
N2      = N2 / sum(N2);
jointii = N1'*N2;
ind     = find( jointi>0 & jointii>0 );
ent    = sum( sum( jointi(ind) .* log(jointi(ind)./jointii(ind)) ) );