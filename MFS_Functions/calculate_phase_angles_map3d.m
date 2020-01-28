function [phaseangles]= calculate_phase_angles_map3d(potentials)

period=1000/11;
potentials=potentials';
phaseangles=zeros(size(potentials))

for i=1:size(potentials,1)
    
    [Electrogram_Phase] = Unipolar2Phase_Electrogram(potentials(i,:),period);
    phaseangles(i,:)=Electrogram_Phase;
end
end