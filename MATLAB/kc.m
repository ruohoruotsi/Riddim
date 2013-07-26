function [y,c,cw] = kcc( x, nc, dom, mth, ss, wn, wn2, nrmf, sprf, tp, cs, nl)

% Decode sizes
sz = ss(1);
hp = ss(2);
pd = ss(3);

% Cell-ize method
if ~iscell( mth)
	m{1} = mth;
	mth = m;
end

% Vectorize nc if it isn't a vector
if length( nc) == 1 & length( mth) > 1
	nc = nc * ones( length( mth), 1);
end

% Do ST analysis
fprintf( 'ST analysis ... ');
if tp < 6
	s = stt( x, sz, hp, pd, wn, tp);
else
	s = fbtr( x, sz, hp, pd);
end

% Preprocess spectra according
p = 1;
if tp == 0 % Complex spectra
	switch nl
	case 'abs'
		p = angle( s);
		p = (cos(p) + sqrt(-1)*sin(p));
		s = abs( s);
	case 'none'
		1;
	otherwise
		error( 'Unknown function ...');
	end
else % Real spectra
	switch nl
	case 'abs'

	  % no good ...
	  % p = sign( s);
	   p = angle(s);
	   s = abs( s);
	case 'none'
		1;
	otherwise
		error( 'Unknown function ...');
	end
end
fprintf( 'done\n');

% Decide on orientation
if dom == 'f'
	c = s.';
	p = p';
else
	c = s;
end

% Preprocess data
if nrmf
	for i = 1:rows( c)
		mn(i) = mean( c(i,:));
		c(i,:) = c(i,:) - mn(i);
		mx(i) = max( abs( c(i,:)));
		c(i,:) = c(i,:) / mx(i);
	end
end

% Start analysis loop
wf = eye( rows( c));

for i = 1:length( nc)

	% Analyze ...
	switch mth{i}
		case 'pca'
                        fprintf( 'PCA ... ');
			[uu,ss,w] = svd( c * c');
			w = ss * w';
		case 'gca'
                        fprintf( 'GCA ... ');
			w = sqrtm( c * c');
		case 'fastica'
                        fprintf( 'FastICA ... ');
			[iw, w] = fastica( c, 'approach', 'symm', 'verbose', 'off', 'displayMode', 'off', 'numOfIC', nc(i));
		case 'fasticad'
                        fprintf( 'FastICA ... ');
			[iw, w] = fastica( c, 'verbose', 'off', 'displayMode', 'off', 'numOfIC', nc(i));
		case 'jade'
                        fprintf( 'JADE ... ');
			w = pinv( jade( c, nc(i)));
		case 'bell'
                        fprintf( 'Bell ... ');
			w = bello( c, 5, 10, .0001, 0);
		case 'null'
			w = eye( rows( c));
		otherwise
			error( 'Unknown step ... ');
	end

	% Select most important components as asked
	if rows( w) ~= nc(i)
		switch cs
		case 'maxvariance'
			[ii,ii] = sort( -var( w'));
		case 'minvariance'
			[ii,ii] = sort( var( w'));
		case 'maxkurtosis'
			[ii,ii] = sort( -kurtosis( w'));
		case 'minkurtosis'
			[ii,ii] = sort( kurtosis( w'));
		otherwise
			error( 'Unknown component selection type ... ');
		end
		w = w(ii(1:nc(i)),:);
	end
	fprintf( 'done\n');

	% Get the new components
	c = w * c;

	% Remember overall change
	wf = w * wf;
end
nc = nc(end);

% Get the component weights
cw = pinv( wf);

% Postprocess data
if nrmf
	for i = 1:rows( c)
		c(i,:) = c(i,:) * mx(i);
		c(i,:) = c(i,:) + mn(i);
		c(i,:) = abs( c(i,:) .^ 1.2);
		c(i,:) = filter( hanning( 3), 1, c(i,:));
	end
end

% Reconstruct
fprintf( 'Reconstruction ... ');
[m,n] = size( cw);
if tp < 6
	y = zeros( cols( cw), (cols(s)+1)*hp+sz+pd);
else
	y = zeros( cols( cw), length( x));
end
for i = 1:n
	% Get spectrum of one component
	ys = ( cw(:,i) * c(i,:) ) .* p;
	if dom == 'f', ys = ys.'; end

	% Synthesize (if needed with phase reconstruction)
	if tp < 6
		if ~sprf
			y(i,:) = istt( ys, hp, pd, wn2, tp);
		else
			y(i,:) = spr( ys, hp, pd, 20);
		end
	else
		y(i,:) = sum( ys, 1);
	end
      end

fprintf( 'done\n');
