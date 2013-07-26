function [pc,latent,explained] = pcacov(x)
% PCACOV  Principal Component Analysis using the covariance matrix.
%   [PC, LATENT, EXPLAINED] = PCACOV(X) takes a the covariance matrix,
%   X, and returns the principal components in PC, the eigenvalues of
%   the covariance matrix of X in LATENT, and the percentage of the
%   total variance in the observations explained by each eigenvector 
%   in EXPLAINED.

%   Reference: J. Edward Jackson, A User's Guide to Principal Components
%   John Wiley & Sons, Inc. 1991 pp. 1-25.

%   B. Jones 3-17-94
%   Copyright(c) 1993-96 by The MathWorks, Inc.
%   $Revision: 2.4 $  $Date: 1996/03/05 15:06:49 $

[u,latent,pc] = svd(x);
latent = diag(latent);

totalvar = sum(latent);
explained = 100*latent/totalvar;
