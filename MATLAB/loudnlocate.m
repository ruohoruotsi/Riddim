function j=loudnlocate(xx,n,x)

% Given an aray xx[1..n], and given a value x, returns a value j 
% such that x is between xx[j] and xx[j+1]. xx must be monotonic,
% either increasing or decreasing. j=0 or j=n is returned to indicate
% that x is out of range.
% Numerical recipies p.98 

jl=0;
ju=n+1;

while (ju-jl) > 1
  jm=(ju+jl)/2;
  if ((xx(n) > xx(1)) == (x > xx(round(jm))))
    jl=jm;
  else
    ju=jm;
  end
end
j=round(jl);
return

