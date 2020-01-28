function [mfsEGM]=solve_MFS(c_x,c_y,c_z,c_ele_1,c_ele_2,c_ele_3,a_x,a_y,a_z,a_ele_1,a_ele_2,a_ele_3, catheter_potential)

catheter_points=[c_x;c_y;c_z];
catheter_mesh=[c_ele_1;c_ele_2;c_ele_3];

atrial_points=[a_x;a_y;a_z];
atrial_mesh = [a_ele_1;a_ele_2;a_ele_3];



%% Load Data

triCath = triangulation(double(catheter_mesh)',catheter_points');
triLA = triangulation(double(atrial_mesh)', atrial_points');

badChannels = true(max(size(c_x)),1);

Geometries = geometryObject(triLA,triCath,'ratioAtria', 1.16, 'ratioBasket',0.92);

transferMFS = transferObject(Geometries,'mfs',badChannels); 

Signals = catheter_potential';

CathSignal = signalObject(Signals,'samp',3000,'tri',triCath,'badLeads',badChannels);

[mfsEGM, regParam] = transferMFS.solveInverse(CathSignal.processedSignal,zeros(size(CathSignal.processedSignal)),'reg',0.15);

end

