classdef geometryObject
    % Custom class to manage catheter and atrial geometries
    %   GEOMETRYOBJECT provides methods to manipulate a triangular geometries.
    %   
    % Input: 
    % - triAtria triangulation mesh of the atria
    % - triBasket tringulation mesh of the catheter
    % 
    % Optional inputs:
    % - 'ratioAtria': ratio to inflate Atria for MFS method (Default = 1.2) 
    % - 'ratioBasket': ratio to deflate catheter for MFS method (Default = 0.8) 

    
    %% Properties
    properties
        triBasket;
        triAtria;
        neighsAtria;
        neighsBasket;
        normAtria;
        normBasket;
        
    end
     properties (GetAccess = 'public', SetAccess = 'public')
        ratioAtria;
        ratioBasket;
    end

    methods
        %% Constructor function
        function obj = geometryObject(atria,basket,varargin)
            
            if isa(atria,'triangulation')
                obj.triAtria  = atria;
            else
                error('atria tri coordinates not valides');
            end
            if isa(basket,'triangulation')
                obj.triBasket = basket;
            else
                error('basket tri coordinates not valides');
            end
            
            opts = parseArgsToStruct(varargin, ...
                {'ratioAtria', 'ratioBasket'}, ...
                {'ratioAtria', 'ratioBasket'}, ...
                {1.2, 0.8});
            fnames = fieldnames(opts);
            for i = 1:length(fnames), obj.(fnames{i}) = opts.(fnames{i}); end
            
            % Check triangulation orientation (normals pointing inwards)
            [checkAtria, checkBasket] = obj.isCW;
            if checkAtria, 
                obj.triAtria = triangulation(obj.triAtria.ConnectivityList(:, [1 3 2]), obj.triAtria.Points);
                warning('Permuted heart triangulation matrix');
            end
            if checkBasket, 
                obj.triBasket = triangulation(obj.triBasket.ConnectivityList(:, [1 3 2]), obj.triBasket.Points);
                warning('Permuted torso triangulation matrix');
            end
            
            % Compute neighbors and normal vectors
            obj.neighsAtria = neighbors(obj.triAtria);
            obj.neighsBasket = neighbors(obj.triBasket);
            obj.normBasket = computeNormals(obj.triBasket);           
            obj.normAtria = computeNormals(obj.triAtria);
            
        end
        function obj = centerMe(obj)
            % Center on the mean heart position;
            meanX = mean(obj.triAtria.Points, 1);
            
            % Centre the atria and basket
            obj.triAtria = triangulation(obj.triAtria.ConnectivityList, ...
                obj.triAtria.Points - repmat(meanX, size(obj.triAtria.Points, 1), 1));
            obj.triBasket = triangulation(obj.triBasket.ConnectivityList, ...
                obj.triBasket.Points - repmat(meanX, size(obj.triBasket.Points, 1), 1));

        end
        
       
    end
end

