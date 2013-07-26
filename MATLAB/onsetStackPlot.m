function stackplot( m, times, o)

if nargin == 2
  o = 1;
end

cla
if o
  fill( [1 1 cols(m) cols(m)], [rows(m)+.5 .5 .5 rows(m)+.5], [1 1 1]*.75)
else
  fill( [rows(m)+.5 .5 .5 rows(m)+.5], [1 1 cols(m) cols(m)], [1 1 1]*.75)
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% start IO havoc
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% onset_vector = zeros(length(y), 1);
%
% for j = 1:length(times),
%  onset_vector(times(j)) = 2;
% end
%  
% t = (0:length(y)-1)';
% y = y+1;
%
% figure;
% plot(t,y, t, onset_vector, 'r'); 
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% end IO havoc . . . sorta . . .
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


hold on
for i = 1:rows( m)
  p = m(i,:) - min( m(i,:));
  p = .5*(p / max( abs( p)) - .5);

  if o
    fill( [1 1 length(p) length(p)], [i+.45 i-.45 i-.45 i+.45], [1 1 1]*.9)
    plot( p+i)
  else
    fill( [i+.45 i-.45 i-.45 i+.45], [1 1 length(p) length(p)], [1 1 1]*.9)
    plot( p+i, 1:length( p), p+i, abs(m(i,:)))
  end

end


if o
  set( gca, 'ytick', 1:rows(m))
  axis( [1 length( p) .5 rows(m)+.5])
else
  set( gca, 'xtick', 1:rows(m))
  axis( [.5 rows(m)+.5 1 length(p)])
end
hold off
