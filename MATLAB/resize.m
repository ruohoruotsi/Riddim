function resize(pct,hdl)


% Function resize(Pct,Hdl) -- Resize a plot or a subplot.
%  The user specifies the percentage  increase (positive)
%  or decrease (negative) in the size of the plot or subplot. 
%  a specific handle  for the plot can be input, the default
%  is Hdl = gca. Must be used separately for each subplot.

% David B. Enfield (Dept of Commerce/NOAA/AOML)

dp = pct/100;

if nargin == 2
  p = get(hdl,'position');
  set(hdl,'position',[p(1)-dp,p(2)-dp,p(3)+2*dp,p(4)+2*dp]);
else
  p = get(gca,'position');
  set(gca,'position',[p(1)-dp,p(2)-dp,p(3)+2*dp,p(4)+2*dp]);
end