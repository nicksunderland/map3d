function [mdVdT, timeLag]  = detectActivation(obj)
% DETECTACTIVATION For each Egm of the input matrix, this function computes the maximum dV/dT 
% 
% ------------------------------------------------------------------------
% OUTPUTS
% mdVdT -> the position of the maximum dV/dt for each EGM, the earliest 
% value being 0
% timeLag -> time lag of the earliest dV/dt to the first time sample
% ------------------------------------------------------------------------

nChannel = obj.nChannel;
data = obj.processedSignal';
scale = obj.samp;
mdVdT = zeros(1,nChannel);
for iChannel=1:nChannel
    FilteredSig = filter([1 -1],1,data(iChannel,:));  
    [~,mdVdT(iChannel)] = min(FilteredSig(2:end));
end


timeLag = min(mdVdT);
mdVdT = (mdVdT-timeLag)/scale;

obj.markers{size(obj.markers,2)+1} = mdVdT;
obj.markersLabel{size(obj.markersLabel,2)+1} = 'mdVdT';
obj.markers{size(obj.markers,2)+1} = timeLag;
obj.markersLabel{size(obj.markersLabel,2)+1} = 'timeLag';


end







