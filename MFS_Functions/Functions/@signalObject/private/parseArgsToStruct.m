function outstruct = parseArgsToStruct( inputcell, validargs, outvars, defaultvals )
%PARSEARGS parses argument/value pairs and returns a structure in which
%they are descibed in the form of outstruct.name = value
%
% JD 05/05/15

    nVArgs = length(validargs);
    if nargin < 3 || isempty(outvars), outvars = validargs; end
    if nargin < 4 || isempty(defaultvals), defaultvals = cell(nVArgs, 1); end
    
    
    assert(iscell(inputcell) && iscell(validargs) && iscell(outvars) && iscell(defaultvals), 'Invalid inputs');
    nIArgs = length(inputcell);
    assert(mod(nIArgs,2) == 0, 'Invalid number of input name/value pairs');
    nIArgs = nIArgs/2;
    
    usedmask = false(1, nIArgs);
    
    outstruct = struct();
    
    % Fill structure with input name/value pairs
    for i  = 1:nVArgs
        imatch = strcmpi(validargs(i), inputcell(1:2:end));
        if any(imatch)
            % Use input parameter
            usedmask(imatch) = true;
            outstruct.(outvars{i}) = inputcell{find(imatch, 1)*2};
        else
            % Fill with default value
            currval = defaultvals{i};
            if isa(currval, 'function_handle'), currval = currval(); end
            outstruct.(outvars{i}) = currval;
            
        end
    end
    
    % Detect unused arguments and issue warnings
    if any(~usedmask)
        for i = find(~usedmask)
            warning('Unknown input argument %s.', inputcell{i*2-1});
        end
    end
end

