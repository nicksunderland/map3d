function [transferA, forwardMatrix] = computeTransfer(obj)
%COMPUTE method - calculate transfer matrix between catheter and atria
%Application Name
%                 
% LB 01-2018

% Define points and normals
basketX = obj.pGeometry.triBasket.Points(obj.pchannelMask,:);
basketNormals = obj.pGeometry.normBasket(obj.pchannelMask,:);
heartX = obj.pGeometry.triAtria.Points;
heartNormals = obj.pGeometry.normAtria;


nptHeart = size(heartX, 1); nptBasket = size(basketX, 1);

if all(obj.pchannelMask)
    basketTri = obj.pGeometry.triBasket.ConnectivityList;
else
    basketTri = convexHull(delaunayTriangulation(basketX));
end



%%  METHOD OF FUNDAMENTAL SOLUTIONS 
if strcmp(obj.pformulation, 'mfs')

    % Calculate center of heart mesh to inflation/deflation of meshes
    heartCentre = mean(heartX, 1);
    
    % Deflate basket mesh and inflate heart mesh for ficticious sources
    heartRatio = obj.pGeometry.ratioAtria;   basketRatio = obj.pGeometry.ratioBasket;
    nptSources = nptHeart + nptBasket;
    
    fictHeart = repmat(heartCentre, nptHeart, 1) + heartRatio * (heartX - repmat(heartCentre, nptHeart, 1));
    fictBasket = repmat(heartCentre, nptBasket, 1) + basketRatio * (basketX - repmat(heartCentre, nptBasket, 1));
    fictSources = [fictHeart ; fictBasket];

    %% Calculate transfer matrix to ficitious sources
    % Dirlichet conditions 
    drlchBasket = repmat(sum(basketX .^ 2, 2),1,nptSources);
    drlchSource = repmat(sum(fictSources' .^ 2, 1),nptBasket,1);
    drlchBasketSource = -2 * basketX * fictSources';
    drlch = 1./sqrt(drlchBasket + drlchSource + drlchBasketSource);
    
    % Neumann conditions
    %                                                                                                                                                                                                                                                                             
    nmnSub1 = basketNormals * fictSources';
    nmnSub2 = repmat(sum(basketX .* basketNormals, 2),1,nptSources);
    nmn = (nmnSub1-nmnSub2).*drlch.^3;
    
    % Combine boundary conditions and add vector for a0
    transferA = [[ ones(nptBasket, 1) drlch ];...
        [zeros(nptBasket, 1)  nmn  ]];
    
    % Compute forward matrix from virtual sources to epicardium
    forHeart = repmat(sum(heartX.^2,2),1,nptSources);
    forSource = repmat(sum(fictSources'.^2,1),nptHeart,1);
    forSourceHeart = -2 * heartX * fictSources';
    forA = 1./sqrt(forHeart + forSource + forSourceHeart);
    forwardMatrix = [ones(nptHeart, 1) forA];
    


%% MFS No Flux: remove the neumann conditions from the implementation
elseif strcmp(obj.pformulation, 'mfsNoFlux') 
    % Calculate center of heart mesh to inflation/deflation of meshes
    heartCentre = mean(basketX, 1);
    
    % Deflate basket mesh and inflate heart mesh for ficticious sources
    heartRatio = obj.pGeometry.ratioAtria;   basketRatio = obj.pGeometry.ratioBasket;
    nptSources = nptHeart + nptBasket;
    
    fictHeart = repmat(heartCentre, nptHeart, 1) + heartRatio * (heartX - repmat(heartCentre, nptHeart, 1));
    fictBasket = repmat(heartCentre, nptBasket, 1) + basketRatio * (basketX - repmat(heartCentre, nptBasket, 1));
    fictSources = [fictHeart ; fictBasket];

    %% Calculate transfer matrix to ficitious sources
    % Dirlichet conditions 
    drlchBasket = repmat(sum(basketX .^ 2, 2),1,nptSources);
    drlchSource = repmat(sum(fictSources' .^ 2, 1),nptBasket,1);
    drlchBasketSource = -2 * basketX * fictSources';
    drlch = 1./sqrt(drlchBasket + drlchSource + drlchBasketSource);
    
    % Combine boundary conditions and add vector for a0
    transferA = [ ones(nptBasket, 1) drlch ];
    
    % Compute forward matrix from virtual sources to epicardium
    forHeart = repmat(sum(heartX.^2,2),1,nptSources);
    forSource = repmat(sum(fictSources'.^2,1),nptHeart,1);
    forSourceHeart = -2 * heartX * fictSources';
    forA = 1./sqrt(forHeart + forSource + forSourceHeart);
    forwardMatrix = [ones(nptHeart, 1) forA];   
    
    
%% STANDARD FORMULATION with LINEAR INTERPOLATION APPROACH (De Munck '92)
elseif strcmp(obj.pformulation, 'bem')
    % Build structure to pass to the toolbox
    % STRUCTURE DEFINITIONS
    % model
    %  .surface{p}           A cell array containing the different surfaces that form the model
    %                        These surfaces are numbered from the outside to the inside
    %      .node              A 3xN matrix that describes all node positions
    %      .face              A 3xM matrix that describes which nodes from one triangle
    %                        Each trianlge is represented by one column of this matrix
    %      .sigma            The conductivity inside and outside of this boundary
    %                        The first element is the conductivity outside and the second the one inside
    %      .cal              The vector describes the calibration of the potential the nodes in this vector
    %                        will be summed to zero in the deflation process.
    
    heartModel = struct('node', heartX', 'face', obj.pGeometry.triAtria.ConnectivityList', 'sigma', [0.22 0]);
    basketModel = struct('node', basketX', 'face', basketTri', 'sigma', [0 0.22]);
    
    completeModel.surface = {basketModel, heartModel};
    [~, transferA] = evalc('bemMatrixPP(completeModel)');
    
    forwardMatrix = eye(size(transferA, 2)); % Does not exist (for compatibility)

end
end


