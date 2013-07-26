function w = bello( x, ep, B, L, w)

if nargin < 5
w = eye( rows( x));
end
P = length( x);

noblocks = fix( P/B);
BI = B*eye( rows( x));
%gf = inline( '1-2*(1./(1+exp(-x)))');

for e = 1:ep
	t = 1;
	for t = t:B:t-1+noblocks*B,
	  u = w*x(:,t:t+B-1); 
	  w = w+L*(BI+(1-2*(1./(1+exp(-u))))*u')*w;
%	  w = w+L*(BI+gf(u)*u')*w;
	end
end

function y = gf( x)

%a = 1-2*(1./(1+exp(-abs(x))));
a = tanh( -abs( x));
p = angle( x);
y = a .* (cos(p) + i*sin(p));
