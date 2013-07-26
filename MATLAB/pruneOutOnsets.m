function [acceptIds] = pruneOutOnsets( louds, times, loudthresh, mininterv)

% PRUNE OUT ONSETS
% 
% Onsets with loudness below 'loudthresh' or closer than 'loudthresh' to
% a louder onset are pruned out from the input onset vectors.
%
% ----old comm's
%
% peakids = peakpickheight( data, maxcount, <minheight>)
%   peakids  : vector of peakids' indices in vector 'data'
%   data     : data vector to investigate
%   maxcount : Maximum number of peakids to give as output
%   minheight (optional) : Minimum height of an accepted peak as a relative
%              value between the maximum and minimum data values.
%              Value must be >= 0 and <= 1.
%              If not given, minheight is set to 0.
% ----old comm's
%
% Programmed 29.6.1999. Anssi Klapuri, klap@cs.tut.fi.


onsetCount=length(louds);

% ----- Initialize 'acceptIds' to contain all onsets */
acceptIds=0:length(louds)-1;

% ----- Delete too-close-to-a-louder onsets */
idb=0;
for ida=0:onsetCount-1,
  if louds(ida+1) >= loudthresh,
    % -- check earlier onsets */
    belowOK=1;
    dev=1;
    while( (ida-dev) >= 0 &...
	  (times(ida+1)-times(ida-dev+1)) <mininterv), % checks next onset for closeness
      if( louds(ida+1) <= louds(ida-dev+1)),
	belowOK=0;
      end;
      dev=dev+1;
    end;
    
    % IO note, if the onsets time(ida+1) onset is less than the time(ida)onset
    % then set belowOk to zero, this effectively allows you to discard this onset
    % and move to the next
    
    % -- check later onsets */
    aboveOK=1;
    dev=1;
    while( (ida+dev) < onsetCount &...
	  times(ida+dev+1)-times(ida+1) <mininterv),
      if( louds(ida+1) <=louds(ida+dev+1)),
	aboveOK=0;
      end;
      dev=dev+1;
    end;
    
    % -- if both are OK, accept the onset, use Matlab indices (+1) */
    if( belowOK & aboveOK ),
      acceptIds(idb+1)=acceptIds(ida+1) + 1;
      idb=idb+1;
    end;
  end;
end;

acceptIds = acceptIds(1:idb)';
