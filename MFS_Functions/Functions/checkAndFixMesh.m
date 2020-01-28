function [ tri,  newtri] = checkAndFixMesh( tri) %#codegen
%CHECKANDFIXMESH This function checks the mesh and fixes it if necessary
%
% LB 01/12/14
    nFaces = size(tri.ConnectivityList, 1);   
    nPts = size(tri.Points, 1);   
    
%% Check that some faces are not used twice
    sorted = sort(tri.ConnectivityList, 2);
    if nFaces ~= size(unique(sorted, 'rows'), 1)
        warning('Face definition is incorrect: some faces are mentioned twice... Trying to fix mesh ...');
        % Remove the double faces
        [~, toremove, ~] = unique(sorted, 'rows');
        toremove = setdiff(1:nFaces, toremove);
        newtri = tri.ConnectivityList;
        newtri(toremove, :) = [];
        disp('... Extra faces removed');
        tri = triangulation(newtri, tri.Points);
        nFaces = size(tri.ConnectivityList, 1);
    end

%% Check that the surface is closed
    if ~isempty(freeBoundary(tri))
        warning('Mesh is not closed... Trying to patch surface...');
        [tmpTri, newtri] = closeSurfaceMesh(tri);
        leftovers = freeBoundary(tmpTri);
        if ~isempty(leftovers)
            if size(leftovers, 1) == 1
               % There is an extra facet that should not exist and is isolated...
               warning('Extra facet present, probably should not be there. Trying to fix...');
               [extra1, ~] = find(tmpTri.ConnectivityList == leftovers(1, 1));
               [extra2, ~] = find(tmpTri.ConnectivityList == leftovers(1, 2));
               extra1 = intersect(extra1, extra2);
               if length(extra1) == 1
                   newtri = tmpTri.ConnectivityList;
                   newtri(extra1,:) = [];
                   tmpTri = triangulation(newtri, tri.Points);
                   if ~isempty(freeBoundary(tmpTri))
                       error('Could not fix mesh... try to fix manually');
                   else
                       disp('... Mesh patching and extra vertex suppression successful');
                       tri = tmpTri;
                   end
               else
                   error('Mesh patch failed, fix mesh manually.');   
               end
            else
                error('Mesh patch failed, fix mesh manually.');
            end
        else
            disp('... Mesh patching successful');
            tri = tmpTri;
        end 
    end
    

%% Check that the vertices are oriented the same way
    tri = checkMat(tri);
       
    nFaces = size(tri.ConnectivityList, 1);
    
%% Check that normal vectors are globaly pointing inwards
    midpoint = mean(tri.Points, 1);
    ptvectors = tri.Points - repmat(midpoint, nPts, 1);
    scalprod = zeros(nFaces, 1);

    for i = 1:nFaces
        f = tri.ConnectivityList(i,:);
        edge1 = tri.Points(f(3),:)-tri.Points(f(1),:);
        edge2 = tri.Points(f(2),:)-tri.Points(f(1),:);
        n = cross(edge1, edge2);
        meanvect = mean(ptvectors(f, :), 1);
        scalprod(i) = dot(meanvect, n);
    end
    if mean(scalprod) > 0
        % Flip all the faces
        warning('Normal vectors were pointing outwards - flipping them !');
        tri = flipFaces(tri, (1:nFaces));
    end
    %if mean(scalprod) < 0
            % Flip all the faces
    %        warning('Normal vectors were pointing inwards - flipping them !');
    %        tri = flipFaces(tri, (1:nFaces));
    %end 
    
    disp('-- Mesh check successful --');
end

function tri = checkMat(tri)
% CHECK ALL EDGES ARE ORIENTED PROPERLY
    nPts = size(tri.Points, 1); nFaces = size(tri.ConnectivityList, 1);
    checkmatrix = zeros(nPts);
    for i=1:nFaces % Check that everyone is numbered the same way, only once
        checkmatrix(tri.ConnectivityList(i,1), tri.ConnectivityList(i,2)) ...
            = checkmatrix(tri.ConnectivityList(i,1), tri.ConnectivityList(i,2)) + 1;
        checkmatrix(tri.ConnectivityList(i,2), tri.ConnectivityList(i,3)) ...
            = checkmatrix(tri.ConnectivityList(i,2), tri.ConnectivityList(i,3)) + 1;
        checkmatrix(tri.ConnectivityList(i,3), tri.ConnectivityList(i,1)) ...
            = checkmatrix(tri.ConnectivityList(i,3), tri.ConnectivityList(i,1)) + 1;
    end
    if (max(checkmatrix(:)) > 2)
        error('Some edges are referenced more than twice !!')
    end
    
    % Find the regions that need flipping ...
    region = {};
    [r, c] = find(checkmatrix == 2);
    boundaries = [r c];
    boundaries = sort(boundaries, 2);
    if isempty(boundaries), return;
    else warning('Some faces are not oriented properly, trying to flip them...');
    end
    
    numBound = 0;
    while(size(boundaries, 1) > 2)
        numBound = numBound+1;
        bound1 = [];     bound2 = [];
        % Initiate the region-growing algorithm by finding the faces 
        % connected to the first boudary element
        connfaces = edgeAttachments(tri, boundaries(1, :));
        region1 = connfaces{1}(1);  region2 = connfaces{1}(2); 
        growing1 = 1;   growing2 = 1;
        
        % Iterative call to the region growing algorithm
        while growing1 && growing2
            [region1, bound1, growing1] = growRegion(tri, region1, boundaries);
            [region2, bound2, growing2] = growRegion(tri, region2, boundaries);
        end
        
        % Once a closed region has been found on either side of the edge,
        % stop and remove it
        if ~growing1
            boundaries = setdiff(boundaries, bound1, 'rows');
            region{numBound} = region1;
        else
            boundaries = setdiff(boundaries, bound2, 'rows');
            region{numBound} = region2;
        end
    end
    
    if ~isempty(boundaries), error('Could not flip faces properly'); end
    
    
    % Once the faces have been found, flip them
    for i =1:length(region), tri = flipFaces(tri, region{i}); end
    fprintf('... Done flipping %d regions\n', length(region));
    
end

function  [outregion, border, growing] = growRegion(tri, region, boundaries)
    % RECURSIVE REGION GROWING FUNCTION
    % 1) Find the border corresponding to our region
    % a) Find all edges first
    border = arrayfun(@(x) [tri.ConnectivityList(x, 1); tri.ConnectivityList(x, 2); ...
        tri.ConnectivityList(x, 2); tri.ConnectivityList(x, 3); ...
        tri.ConnectivityList(x, 3); tri.ConnectivityList(x, 1)], region, 'UniformOutput',0);
    border = cell2mat(border);
    border = reshape(border, 2, [])';
    border = sort(border, 2);

    % b) Find edges mentionned twice (which therefore do not lie on the border)
    [~, ia, ~] = unique(border, 'rows');
    ia = setdiff(1:size(border, 1), ia);

    % c) Remove them to obtain only those mentionned once
    border = setdiff(border, border(ia, :), 'rows');
    
    % d) Remove from these edges the ones corresponding to region boundaries
    growborder = setdiff(border, boundaries, 'rows');
    
    if isempty(growborder)
        growing = 0;
        outregion = region;
    else
        growing = 1;
        
        % 2) Find and add the new faces using the grow border
        newfaces = edgeAttachments(tri, growborder);
        newfaces = cell2mat(newfaces);
        newfaces = newfaces(:);
        outregion = unique([newfaces; region(:)]);
    end    
end

function tri = flipFaces(tri, facelist)
    % FACEFLIPPING FUNCTION
    newtri  = tri.ConnectivityList;
    newtri(facelist, :) = newtri(facelist,[1 3 2]);
    tri=triangulation(newtri, tri.Points);
end
        