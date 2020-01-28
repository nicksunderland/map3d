function [outnargs, outnames, outvalues] = parseArgsDefault( inputcell, validargs, outvars )
%PARSEARGS parses argument/value pairs
%
% JD 05/05/15

    if nargin < 3 || isempty(outvars), outvars = validargs; end
    assert(iscell(inputcell) && iscell(validargs) && iscell(outvars), 'Invalid inputs');
    nargs = length(inputcell);
    assert(mod(nargs,2) == 0, 'Invalid number of input name/value pairs');
    outnargs = nargs/2;
    
    outnames = cell(1, outnargs); outvalues = cell(1, outnargs);
    for i = 1:outnargs
        imatch = strcmpi(inputcell{i*2-1}, validargs);
        if any(imatch)
            outnames(i) = outvars(find(imatch, 1));
            outvalues(i) = inputcell(i*2);
        else
            error('Unknown input option (%s)\n', inputcell{i});
        end
    end
end

