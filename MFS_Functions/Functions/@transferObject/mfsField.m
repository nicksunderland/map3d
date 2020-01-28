function [ field ] = mfsField( obj, insig, varargin )
%MFSFIELD Method outputs the MFS potential field computed using the
%specified arguments
%
% JD 07/05/15
    assert(nargin > 1, 'Please provide valid input signal');
    assert(size(insig, 1) == size(obj.pciCase.triTorso.X, 1), 'Invalid input signal size');
    
    assert(nargin > 1, 'Please provide valid input signal');
    assert(size(insig, 1) == size(obj.pciCase.triTorso.X, 1), 'Invalid input signal size');
    % Default parameters
    params.regval = 10^-1;
    params.regmeth = 'tikh';
    
    % Parse input arguments
    [nvargs, vargnames, vargvals] = parseArgs(varargin, ...
        {'method', 'reg'}, ...
        {'regmeth', 'regval'});
    for i = 1:nvargs, params.(vargnames{i}) = vargvals{i}; end

    % Invert system
    field = obj.invert(insig(obj.channelMask, :), params.regmeth, params.regval);
end

