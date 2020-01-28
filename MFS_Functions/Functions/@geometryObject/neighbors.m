function [ neighbors, distneighs ] = neighbors(tri)
%NEIGHBORS Calculate neighbors for all points on the mesh
%
% Inputs:
%       tri: input mesh
%
% LB 01/2018

    neighbors = cell(size(tri.Points, 1), 1);
    if nargout > 1, distneighs = cell(size(tri.Points, 1), 1); end
    for i = 1:size(tri.ConnectivityList, 1)
        vertices = sort(tri.ConnectivityList(i, :), 2);
        
        % List pairs of points
        points = [[vertices(1) vertices(2)]; ...
                  [vertices(2) vertices(3)]; ...
                  [vertices(1) vertices(3)]];

        % Add the ones left to the neighbors list
        for j = 1:size(points, 1)
            neighbors{points(j, 1)} = [neighbors{points(j, 1)} points(j, 2)];
            neighbors{points(j, 2)} = [neighbors{points(j, 2)} points(j, 1)];
        end
        
        % Compute distances if necessary
        if nargout > 1
            for j = 1:size(points, 1)
                dist = tri.Points(points(j, 1), :) - tri.Points(points(j, 2), :);
                dist = sqrt(sum(dist.^2));
                distneighs{points(j, 1)} = [distneighs{points(j, 1)} dist];
                distneighs{points(j, 2)} = [distneighs{points(j, 2)} dist];
            end
        end
        
    end
    if nargout <= 1
        neighbors = cellfun(@unique, neighbors, 'UniformOutput', 0);
    else
        [neighbors, uia] = cellfun(@unique, neighbors, 'UniformOutput', 0);
        distneighs = cellfun(@(x,y) x(y), distneighs, uia, 'UniformOutput', 0);
    end    
end
