

clear;
rnd     = 16;
i1      = rand(1,256);
i2      = i1 + 0.1*rand(1,256);


i1      = round( i1 * (255/rnd)/max(i1) );
i2      = round( i2 * (255/rnd)/max(i2) );
dim     = size( i1, 2 );
joint   = zeros( 256/rnd + 1 );
for i = 1 : dim
        joint( i1(i)+1, i2(i)+1 ) = joint( i1(i)+1, i2(i)+1 ) + 1;
end
jointi  = joint / sum(sum(joint));


%%% COMPUTE ENTROPY
x1      = i1;
[N1,X1] = hist( x1, rnd+1 );
N1      = N1 / sum(N1);
x2      = i2;
[N2,X2] = hist( x2, rnd+1 );
N2      = N2 / sum(N2);
jointii = N1'*N2;
ind     = find( jointi>0 & jointii>0 );
ent2    = sum( sum( jointi(ind) .* log(jointi(ind)./jointii(ind)) ) )