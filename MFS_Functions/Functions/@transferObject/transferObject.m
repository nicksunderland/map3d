classdef transferObject
    % Custom class to computer transfer matrix between atria and catheter
    %   TRANSFEROBJECT provides methods to compute transfer matrices.
    %   
    % Input: 
    % - geoObj created using geometryObject class

    % 
    % Optional inputs:
    % - formulation of transfer matrix computation (default = MFS)
    % - channelMask for the bad leads (default = no bad channels)
    
    %% Properties
    properties (Constant, Access = private)
        vformulation = {'mfs','mfsNoFlux','bem'};
    end
    properties (Dependent, Access = public)
        formulation = [];
        geoObj = [];
        channelMask = [];
        
    end
    properties (Access = private)
        %heartX; heartTri; basketX; basketTri;
        triAtria; triBasket;
        heartNorm; basketNorm;
        pformulation = [];
        pGeometry = [];
        pchannelMask = []; 
        virtualForward = [];
        matU = [];
        vectS = []
        matV = [];
        transfer = [];
    end
    
    methods
        %% Constructor function
        function obj = transferObject(geoObj, formulation, channelMask)
            assert(nargin > 0 && isa(geoObj, 'geometryObject'), 'Invalid input geometryObject');
            if nargin < 2 || isempty(formulation), formulation = 'mfs'; end
            if nargin < 3 || isempty(channelMask), channelMask = true(size(geoObj.triBasket.Points, 1), 1); end
            
            assert(any(strcmpi(formulation, obj.vformulation)), 'Invalid input formulation');
            channelMask = logical(channelMask(:));
            assert(length(channelMask) == size(geoObj.triBasket.Points, 1), 'Invalid channel mask size');
            
            obj.pGeometry = geoObj;
            obj.pGeometry = geoObj.centerMe();
            obj.pformulation = formulation;
            obj.pchannelMask = channelMask;
            
            
            % Compute the transfer matrix
            %%fprintf('>> Computing transfer matrix using %s formulation...\n', formulation);
            [obj.transfer obj.virtualForward] = obj.computeTransfer();

        
        end
        
        %% Set/get geoObj
        function geoObj = get.geoObj(obj)
            geoObj = obj.pGeometry;
        end
        
        %% Set/get formulation
        function formulation = get.formulation(obj)
            formulation = obj.pformulation;
        end

        %% Set/get channelMask
        function channelMask = get.channelMask(obj)
            channelMask = obj.pchannelMask;
        end
        
        function obj = set.channelMask(obj, inchannelmask)
            assert(nargin > 1 && length(inchannelmask) == size(geoObj.triHeart.Points, 1), 'Invalid input channel mask');
            obj.pchannelMask = inchannelmask;
            
            % Re-compute the transfer matrix
            fprintf('>> Re-computing transfer matrix using %s formulation...\n', obj.pformulation);
            [obj.transfer obj.virtualForward] = obj.computeTransfer();
        end
    end
end

