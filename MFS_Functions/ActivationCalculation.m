function [activation]=ActivationCalculation(c_x,c_y,c_z, potentials)

LA=[c_x;c_y;c_z]';

inverse_result=potentials';

grad_inverse=gradient(inverse_result);


activation=zeros(size(inverse_result,1),1);

for i=1:size(inverse_result,1)
    
    
    [maxtab, mintab] = peakdet(grad_inverse(i,:), 0.1*max(grad_inverse(i,:)));
    
    min_grad=min(grad_inverse(i,:));
    min_thres=min_grad+0.2*std(inverse_result(i,:));
    
    
    pp=(mintab(:,2)<min_thres);
    
    if sum(pp)==1
        
        [~,q]=ismember(1,pp);
        activation(i,1)=mintab(q,1);
    end
    
    
end

%%%order neighbouring points

[tri,tnorm]=MyRobustCrust(LA);

edges = [tri(:,1:2);tri(:,[1 3]);tri(:,2:3)];
edges = sort(edges,2);
edges = sortrows(edges);
edges = unique(edges,'rows');


count=0;
for i=1:size(edges,1)
    if (activation(edges(i,1),1)==0)||(activation(edges(i,2),1)==0)
        count=count+1;
        pairs(count,:)=edges(i,:);
        
    end
end


B=zeros(size(pairs,1), size(activation(activation==0),1));
time_Lag=zeros(size(pairs,1),1);

for i=1:size(pairs,1)
    [corrCoeff,timeLags] = xcorr(inverse_result(pairs(i,1),:),inverse_result(pairs(i,2),:),6,'coeff');
    [~,whereCorrCoeffMax] = max(corrCoeff);
    time_Lag(i,1) = timeLags(whereCorrCoeffMax);
    
end


count=0;
for i=1:size(activation,1)
    if activation(i,1)==0
        
        count=count+1;
        unknown_index(count,1)=i;
        
    end
end

for i=1:size(pairs,1)
    
    if (activation(pairs(i,1),1)==0)&&(activation(pairs(i,2),1)==0)
        
        [~,q_positive]=ismember(pairs(i,1),unknown_index);
        B(i,q_positive)=1;
        
        [~,q_negative]=ismember(pairs(i,2),unknown_index);
        B(i,q_negative)=-1;
        
    elseif (activation(pairs(i,1),1)==0)&&(activation(pairs(i,2),1)~=0)
        
        [~,q_positive]=ismember(pairs(i,1),unknown_index);
        B(i,q_positive)=1;
        time_Lag(i,1)=time_Lag(i,1)+activation(pairs(i,2),1);
        
    elseif (activation(pairs(i,1),1)~=0)&&(activation(pairs(i,2),1)==0)
        
        [~,q_negative]=ismember(pairs(i,2),unknown_index);
        B(i,q_negative)=-1;
        time_Lag(i,1)=time_Lag(i,1)-activation(pairs(i,1),1);
    end
end


at=lsqr(B,time_Lag);
at=round(at);

for i=1:size(unknown_index,1)
    
    activation(unknown_index(i,1),1)=at(i,1);
    
end

activation = normalize(activation,'range');

end






