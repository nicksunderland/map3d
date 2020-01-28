function [ forward] = forwardCustom( obj, inPts )
%FORWARDTORSO Method calculates the matrix linking the MFS reprensentation
%to a the potential on the input points.

    %% Input management
    assert(ismatrix(inPts) && size(inPts, 2) == 3, 'Points must be 3D, list along lines');
    torsoX = obj.pciCase.triTorso.X(obj.pchannelMask,:);
    heartX = obj.pciCase.triHeart.X;
    
    nptHeart = size(heartX, 1); nptTorso = size(torsoX, 1);
    nptIn = size(inPts, 1);
    nptSources = nptHeart + nptTorso;
    
    %% Inflate torso mesh and contract heart mesh for ficticious sources
    heartRatio = 0.8;   torsoRatio = 1.2;
    meanX = mean(heartX, 1);
    fictHeart = repmat(meanX, nptHeart, 1) + ...
        heartRatio * (heartX - repmat(meanX, nptHeart, 1));
    fictTorso = repmat(meanX, nptTorso, 1) + ...
        torsoRatio * (torsoX - repmat(meanX, nptTorso, 1));
    
    if strcmp(obj.pformulation, 'mfs')
        nptSources = nptHeart + nptTorso;
        fictSources = [fictHeart ; fictTorso];
    elseif strcmp(obj.pformulation, 'truncatedmfs')
        nptSources = nptHeart;
        fictSources = [fictHeart ];
    end    
    
   
    forSqCust = repmat(sum(inPts.^2,2),1,nptSources);
    forSqSource = repmat(sum(fictSources'.^2,1),nptIn,1);
    forSourceCust = inPts*fictSources';
    forA = 1./sqrt(forSqCust+forSqSource - 2*forSourceCust);
    forward = [ones(nptIn, 1) forA];
    
end