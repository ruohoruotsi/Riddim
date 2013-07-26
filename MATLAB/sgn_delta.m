function sgn_delta = sgn_delta(Mi, Mj)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% an implementation of Larry Polansky's "sgn" function from the
% Morphological Metrics paper in the Journal of New Music Research 1996
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if Mi > Mj
  sgn_delta = 1;  
elseif Mi == Mj
  sgn_delta = 0;
else 
  sgn_delta = -1;
end




