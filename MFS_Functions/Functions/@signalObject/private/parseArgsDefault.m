function [outnargs, outnames, outvalues] = parseArgsDefault( inputcell, validargs, defaultValue )
%PARSEARGS parses argument/value pairs
%
% JD 05/05/15

    if nargin < 3 || isempty(defaultValue), outvars = validargs; end
    assert(iscell(inputcell) && iscell(validargs) && iscell(defaultValue), 'Invalid inputs');
    outnargs = length(validargs);
    assert(mod(length(inputcell),2) == 0, 'Invalid number of input name/value pairs');
    
    outnames = cell(1, outnargs); outvalues = cell(1, outnargs);
    argDefault = 0;
    for i = 1:outnargs
        imatch = strcmpi(validargs{i},inputcell);
        if any(imatch)
            argDefault = argDefault+1;
            outvalues{i} = inputcell{imatch+1};
            outnames{i}= validargs{i};
        else
            outvalues{i} = defaultValue{i};
            outnames{i}= validargs{i};
        end
    end

assert(argDefault==length(inputcell),'Invalid inputs');
end