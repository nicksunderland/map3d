function [outsig, lambda] = solveInverse(obj, insig, flux, varargin)
% SOLVEINVERSE solves the inverse problem given the input methods
% Inputs :
%       obj: transferObject class element
%       insig: signal from basket points (potentials)
%       flux: current at the basket points (leave empty if not included in
%       formulation.
% Optional Inputs:
%       method: Method used to compute the inverse -- tikh (default). 
%       reg: regularization parameter itself or the method used to calculate it --
%           l-curve, creso (default), scalar input

    assert(nargin > 1, 'Please provide valid input signal');
    assert(size(insig, 1) == size(obj.pGeometry.triBasket.Points, 1), 'Invalid input signal size');
    % Default parameters
    params.regmeth = 'tikh';
    params.regval = 'creso';
    
    
    [obj.matU,obj.vectS,obj.matV] = csvd(obj.transfer);


    % Parse input arguments
    [nvargs, vargnames, vargvals] = parseArgs(varargin, ...
        {'method', 'reg'}, ...
        {'regmeth', 'regval'});
    for i = 1:nvargs, params.(vargnames{i}) = vargvals{i}; end
    

    % Invert system
    if isempty(flux)
    	[outfield params.regval] = obj.invert(insig(obj.channelMask, :),[], params.regmeth, params.regval);
    else
        [outfield params.regval] = obj.invert(insig(obj.channelMask, :),flux(obj.channelMask, :), params.regmeth, params.regval);
    end
    lambda = mean(params.regval);
    
    outsig = obj.virtualForward * outfield;

end

