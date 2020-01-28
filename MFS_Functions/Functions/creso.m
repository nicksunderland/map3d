function [ lambda ] = creso( matU, vectS, vectB )
%CRESOJD Perform CRESO calculation of regulation parameter for Tikhonov
% Inputs:
%   matU: SVD U matrix of A matrix
%   vectS: Singular values
%   vectB: vector B
%
% LB 01-2018 - see Johnston PR et al. IEEE Trans BioMed Eng 1997
    

%% Input verification
    if min(size(vectB)) > 1, error('Incorrect input B vector'); end

%% Precalculation and settings
    vectAlpha = matU'*vectB;
    vectAlpha = vectAlpha(:);
    S = vectS(:);
    numElem = length(S);
    oneVector = ones(numElem,1);
    
    % Set minimal and maximal regulation parameters
    minReg = 10^-8; maxReg = 10^-2;
     
%% Define CRESO function
    function [cresoval] = local_creso(t)
        t = t.*oneVector;
        cresoval = (S .* vectAlpha ./ (S .^ 2 + t)) .^ 2;
        cresoval = cresoval .* (oneVector - (4 * t ./(S .^ 2 + t)));
        cresoval = -sum(cresoval, 1);
    end

%% Find local minimum of CRESO function
    lambda = fminbnd(@local_creso, minReg, maxReg);
end

