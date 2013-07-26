function [y,dy]=loudnpolint(xa,ya,n,x)

nmax=10;
ns=1;
dif=abs(x-xa(1));
c=zeros(1,nmax);
d=zeros(1,nmax);
for i=1:n
  dift=abs(x-xa(i));
  if dift < dif
    ns=i;
    dif=dift;
  end
  c(i)=ya(i);
  d(i)=ya(i);
end	
y=ya(ns);
ns=ns-1;
for m=1:n-1
  for i=1:n-m
    ho=xa(i)-x;
    hp=xa(i+m)-x;
    w=c(i+1)-d(i);
    den=ho-hp;
    if den==0.0
      display('Error in routine POLINT');
      % This error can occur only if two input xa's are
      % (to within roundoff) identical. 
    end
    den=w/den;
    d(i)=hp*den;	%/* Here the c's and d's are updated. */
    c(i)=ho*den;
  end
  if 2*ns < n-m
    dy=c(ns+1);
  else
    dy=d(ns);
    ns=ns-1;
  end
  y=y+dy;
end
return

% After each column in the tableau is completed, we decide which correction
% c or d, we want to add to our accumulating value of y, i.e. which path to take
% through the tableau-forking up or down. We do this in such a way as to take
% the most "straight line" route through the tableau to its apex,
% updating ns accordingly to keep track of where we are. This route keeps the partial
% approximations centerd (insofar as possible) on the target x. 
% The last dy added is thus the error indication.


