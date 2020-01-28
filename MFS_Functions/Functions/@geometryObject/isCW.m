function [checkAtria checkBasket] = isCW(obj)
%CHECKCW Check geometry is numbered in CCW fashion
% LB 2018

    tri = obj.triAtria.ConnectivityList;
    X = obj.triAtria.Points;
    nTri = size(tri, 1);
    
    triX = zeros(nTri, 3, 3);
    triX(:,:,1) = X(tri(:, 1),:);
    triX(:,:,2) = X(tri(:, 2),:);
    triX(:,:,3) = X(tri(:, 3),:);
    triX = mean(triX, 3);
    
    nVects = cross(X(tri(:, 2),:) - X(tri(:, 1),:), X(tri(:, 3),:) - X(tri(:, 1),:));
    ptVects = triX - repmat(mean(X), nTri, 1);
    
    checkAtria = sum(dot(nVects, ptVects)) < 0;
  
    
    tri = obj.triBasket.ConnectivityList;
    X = obj.triBasket.Points;
    nTri = size(tri, 1);
    
    triX = zeros(nTri, 3, 3);
    triX(:,:,1) = X(tri(:, 1),:);
    triX(:,:,2) = X(tri(:, 2),:);
    triX(:,:,3) = X(tri(:, 3),:);
    triX = mean(triX, 3);
    
    nVects = cross(X(tri(:, 2),:) - X(tri(:, 1),:), X(tri(:, 3),:) - X(tri(:, 1),:));
    ptVects = triX - repmat(mean(X), nTri, 1);
    
    checkBasket = sum(dot(nVects, ptVects)) < 0;

end

