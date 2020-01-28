function [ forward, jacobian ] = forwardTorso( obj, intransform, rigid )
%FORWARDTORSO Method calculates the matrix linking the MFS reprensentation
%to a the potential on the torso, and the jacobian of this.

    %% Input management
    if nargin < 2 || isempty(intransform), 
        intransform = transformMat(eye(3), [0 0 0]); 
    end
    if nargin < 3 || isempty(rigid),
        rigid = false;
    end
    
    global torsoX heartX sourcesX nptHeart nptTorso nptSources forA transform
    transform = intransform;
    
    torsoX = obj.pciCase.triTorso.X(obj.pchannelMask,:);
    heartX = obj.pciCase.triHeart.X;
    
    nptHeart = size(heartX, 1); nptTorso = size(torsoX, 1);
    
    %% Apply transform to the heart mesh
    heartX = transform*heartX;
    
    %% Inflate torso mesh and contract heart mesh for ficticious sources
    meanX = mean(heartX, 1);
    heartRatio = 0.8;   torsoRatio = 1.2;
    fictHeart = repmat(meanX, nptHeart, 1) + ...
        heartRatio * (heartX - repmat(meanX, nptHeart, 1));
    fictTorso = repmat(meanX, nptTorso, 1) + ...
        torsoRatio * (torsoX - repmat(meanX, nptTorso, 1));
    
    if strcmp(obj.pformulation, 'mfs')
        nptSources = nptHeart + nptTorso;
        sourcesX = [fictHeart ; fictTorso];
    elseif strcmp(obj.pformulation, 'truncatedmfs')
        nptSources = nptHeart;
        sourcesX = [fictHeart ];
    end    
    
    
    %% Compute matrix from virtual sources to torso
%     forA = repmat(reshape(torsoX, nptTorso, 1, 3), 1, nptSources) ...
%         - repmat(reshape(fictSources, 1, nptSources, 3), nptTorso, 1);
%     forA = 1./sqrt(sum(forA.^2, 3));
    
    forSqTorso = repmat(sum(torsoX.^2,2),1,nptSources);
    forSqSource = repmat(sum(sourcesX.^2,2)',nptTorso,1);
    forSourceTorso = torsoX*sourcesX';
    forA = 1./sqrt(forSqTorso+forSqSource - 2*forSourceTorso);
    forward = [ones(nptTorso, 1) forA];
    
    %% Compute partial derivative w/ respect to tranformation matrix (Jacobian)
    if nargout > 1
        if rigid
            jacobian = cat(3, rotJacobian(), transJacobian());
        else
            jacobian = cat(3, affineJacobian(), transJacobian());
        end
    end
end

function jacobian = affineJacobian()
    global torsoX heartX sourcesX nptTorso nptSources forA
    jacobian = zeros(nptTorso, nptSources+1, 9);
    
    for i = 1:9
        % Compute unitary transform matrix for each variable
        jtransform_v = zeros(1,12);
        jtransform_v(i) = 1;
        jtransform = transformMat(jtransform_v);

        % Apply unitary transform to heart points
        dHdv = jtransform * heartX;
        dHdv_a = [dHdv ; zeros(nptTorso, 3)];

        % Scalar product
        forGrad1 = torsoX * dHdv_a';
        forGrad2 = repmat(sum(sourcesX .* dHdv_a, 2)', nptTorso, 1);
        forG = -(forGrad1-forGrad2).*forA.^3;
        jacobian(:, :, i) = [zeros(nptTorso, 1) forG];
    end
end


function jacobian = rotJacobian()
    global torsoX heartX sourcesX nptTorso nptSources forA transform
    jacobian = zeros(nptTorso, nptSources+1, 3);
    
    for i = 1:3
        % Compute unitary transform matrix for each variable
        jtransform = transform.dR(i);

        % Apply unitary transform to heart points
        dHdv = jtransform * heartX;
        dHdv_a = [dHdv ; zeros(nptTorso, 3)];

        % Scalar product
        forGrad1 = torsoX * dHdv_a';
        forGrad2 = repmat(sum(sourcesX .* dHdv_a, 2)', nptTorso, 1);
        forG = -(forGrad1-forGrad2).*forA.^3;
        jacobian(:, :, i) = [zeros(nptTorso, 1) forG];
    end
end


function jacobian = transJacobian()
    global torsoX sourcesX nptHeart nptTorso nptSources forA
    jacobian = zeros(nptTorso, nptSources+1, 3);
    
    for i = 1:3
        % Apply unitary transform to heart points
        jtranslation = [0 0 0];
        jtranslation(i) = 1;
        dHdv_a = [repmat(jtranslation, nptHeart, 1) ; zeros(nptTorso, 3)];

        % Scalar product
        forGrad1 = torsoX * dHdv_a';
        forGrad2 = repmat(sum(sourcesX .* dHdv_a, 2)', nptTorso, 1);
        forG = -(forGrad1-forGrad2).*forA.^3;
        jacobian(:, :, 9+i) = [zeros(nptTorso, 1) forG];
    end
end