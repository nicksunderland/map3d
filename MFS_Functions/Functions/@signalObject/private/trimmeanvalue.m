function m = trimmeanvalue(Xin)
% function that computes the mean value, but first remove the min and max 
X = Xin(Xin~=-1);
switch size(X,2)
    case 1
        m = X;
    case 2
        m = mean(X);
    case 3
        ouM = X<max(X);
        m = mean(X(ouM));
    otherwise
        ouM = X<max(X);
        X = X(ouM);
        ouM = X>min(X);
        m = mean(X(ouM));
end
end