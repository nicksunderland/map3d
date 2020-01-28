function [ normals] = computeNormals( tri )
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
nVert = size(tri.Points, 1);
nFace = size(tri.ConnectivityList, 1);
normals = zeros(nVert,3);

for i=1:nFace
    f = tri.ConnectivityList(i,:);
    % Compute the normal to the face
    edge1 = tri.Points(f(3),:)-tri.Points(f(1),:);
    edge2 = tri.Points(f(2),:)-tri.Points(f(1),:);
    n = cross(edge1, edge2);
    n = n/norm(n);
    
    % Sum calculated vector for all adjacent vertices
    for j=1:3
        normals( f(j),: ) = normals( f(j),: ) + n;
    end
end

% Normalize
for i=1:nVert
    n = normals(i,:);
    normals(i,:) = n/norm(n);
end

% Check vectors direction (via sum of dot product vith rays)
rays = tri.Points - repmat(mean(tri.Points, 1), nVert, 1);
sumdot = sum(dot(normals, rays, 2), 1);
if sumdot > 0, normals = -normals; end

end

