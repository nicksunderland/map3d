function [newpoints]= find_touching_points_map3d(vertices,faces,testp,ori_testp)

vertices=vertices';
faces=faces';
testp=testp';
ori_testp=ori_testp';

[in] = intriangulation(vertices,faces,testp);
newpoints=zeros(size(testp));

for i=1:size(testp,1)
    
    if in(i)==1
        
        newpoints(i,:)=testp(i,:)';
    else
        
        distance=point_to_line_distance(vertices, ori_testp(i,:), testp(i,:));
        
        [xs, index] = sort(distance);
        
        source_pts=vertices(sort(index(1:5)),:);
        
        [k,d] = dsearchn(source_pts,testp(i,:));
        
        newpoints(i,:)=source_pts(k,:);
    end
end


end