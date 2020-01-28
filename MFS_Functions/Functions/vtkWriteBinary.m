function vtkWriteBinary(tri,data, filename,fieldname);

% Write vtk file with a data color field

if nargin==3
    for iS=1:size(data,2)
        fieldname{iS} = ['data_' num2str(iS)];
    end
end

nPoint = size(tri.X,1);
nTriangle = size(tri.Triangulation,1);


fid  = fopen(sprintf('%s.vtk',filename),'w');
fprintf(fid,'# vtk DataFile Version 2.0\n');
fprintf(fid,'Written by vtkWrite from Matlab, remi.dubois@espci.fr\n');
fprintf(fid,'BINARY\n');
fprintf(fid,'\n');

fprintf(fid,'DATASET UNSTRUCTURED_GRID\n');

fprintf(fid,'POINTS\t%d\tfloat\n',nPoint);
dataX = [tri.X(:,1),tri.X(:,2),tri.X(:,3)]';
fwrite(fid,dataX,'float','b');

fprintf(fid,'\nCELLS %d %d\n',nTriangle,4*nTriangle);
dataT = [3*ones(size(tri,1),1), tri.Triangulation(:,1)-1,tri.Triangulation(:,2)-1,tri.Triangulation(:,3)-1]';
fwrite(fid,dataT,'int','b');

fprintf(fid,'\nCELL_TYPES\t%d\n',nTriangle);
mat = 5*ones(nTriangle,1);
fwrite(fid,mat,'int','b');


%
if ~isempty(data)
         fprintf(fid,'\nPOINT_DATA %d\n',nPoint);
   
    for iS=1:size(data,2)
        fprintf(fid,'SCALARS %s float 1\n',fieldname{iS});
        fprintf(fid,'LOOKUP_TABLE DEFAULT\n');
        fwrite(fid,data(:,iS),'float','b');
    end
    
end
fclose(fid);
end
