function [mfsEGM]=testest(vertices,faces,testp,ori_testp)

[newpoints]= find_touching_points_map3d(vertices,faces,testp,ori_testp)
%%figure,trisurf(ppp,c_x,c_y,c_z,'FaceAlpha', 1); 

figure,

%plot3(vertices(1,:),vertices(2,:),vertices(3,:),'.');

plot3(testp(1,:),testp(2,:),testp(3,:),'r.');
hold on
trisurf(faces',vertices(1,:),vertices(2,:),vertices(3,:),'edgecolor','none','FaceAlpha', 0.3); 

hold on
newpoints=newpoints';

plot3(newpoints(1,:),newpoints(2,:),newpoints(3,:),'g*');

mfsEGM=0;

end

