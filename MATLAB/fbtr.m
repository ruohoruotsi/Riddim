function y = stb( x, sz, n, sp)

% Default filter specs
if nargin < 3
	n = 4;
end

% Default spacing
if nargin < 4
	sp = 1;
end

% Take care of filter order
if n(1)/2 ~= round( n(1)/2)
	n(1) = n(1) + 1;
	warning( 'Bumped n up one order');
end

% Make filterbank description
cf = linspace( 0, 1, sz+1).^(1/sp);

% Get filterbank coefficients
[b(1,:),a(1,:)] = fil( n, cf(2));
n(1) = n(1)/2;
for i = 2:sz-1
	[b(i,:),a(i,:)] = fil( n, [cf(i) cf(i+1)]);
end
n(1) = n(1)*2;
[b(sz,:),a(sz,:)] = fil( n, cf(sz), 'high');

% Show me the frequency response
warning off
for i = 1:sz fr(:,i) = abs( freqz( b(i,:),a(i,:))); end, plot( 10*log10( fr)), axis( [0 512 -20 5]), drawnow
warning on

% Do forward pass
if rows(x) == 1 | cols( x) == 1
	y = zeros( sz, length(x));
	for i = 1:sz
		y(i,:) = filter( b(i,:), a(i,:), x);
		fprintf('%4d\b\b\b\b', i);
	end
% Do inverse pass
else
	y = 0;
	for i = 1:sz
		y = y + filter( a(i,:), b(i,:), x(i,:));
		fprintf('%4d\b\b\b\b', i);
	end
end

% filter design function
function [b,a] = fil( fp, w, s)

% Decode filter parameters
m = 1;
r1 = .1;
r2 = 20;
o = fp(1);
if length( fp) > 1
	m = fp(2);
end
if length( fp) > 2
	r1 = fp(3);
elseif m == 3
	r1 = 20;
end
if length( fp) > 3
	r2 = fp(4);
end

% Design specified filter
switch( m)
	case 1,
		if nargin > 2
			[b,a] = butter( o, w, s);
		else
			[b,a] = butter( o, w);
		end
	case 2,
		if nargin > 2
			[b,a] = cheby1( o, r1, w, s);
		else
			[b,a] = cheby1( o, r1, w);
		end
	case 3,
		if nargin > 2
			[b,a] = cheby2( o, r1, w, s);
		else
			[b,a] = cheby2( o, r1, w);
		end
	case 4,
		if nargin > 2
			[b,a] = ellip( o, r1, r2, w, s);
		else
			[b,a] = ellip( o, r1, r2, w);
		end
	otherwise,
		error( sprintf( 'Unknown filter type %d', m));
end
