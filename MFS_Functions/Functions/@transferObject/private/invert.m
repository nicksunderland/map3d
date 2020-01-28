function [vectA, lambda] = invert(obj, sig, flux, method, reg )
%INVERT Method solve ECG inverse problem (potential formulation)
%   Formulation Aa = b.
% Inputs :
%       obj: transferObject class element
%       insig: signal from basket points (potentials)
%       flux: current at the basket points (leave empty if not included in
%       formulation.
%       method: method used for regularization -- tikh
%       reg: regularization parameter or method used to calculate it --
%           l-curve (default), creso, scalar input
%
% LB 01-2018

%% Construct b vector
% In MFS formulation, Vector b contains basket potentials (Dirlichet equations) 
% and potentially basket currents (Neumann equations) .
[nPts, nSamp] = size(sig);
if (nPts *2 == size(obj.matU, 1))
    vectB = [sig ; flux];
    % In BEM formulation, Vector b contains basket potentials only
else
    vectB = sig;
end
%hold on
%plot(obj.vectS,'r')

%% TIKHONOV regularization
if strcmp(method, 'tikh')
    
    % If scalar regularization parameter is provided, use it (lambda)
    if isnumeric(reg)
        if length(reg) == nSamp, lambda = reg;
        else
            lambda = reg(1)*ones(nSamp, 1);
            fprintf('Lambda reset - 10^%f\n', log(sqrt(lambda(1))));
        end
        disp('Using provided lambda for Tikh regularization');
        
        
   % L-curve method : draw average l-curve to select reg param
    elseif strcmp(reg, 'l-curve')
        %%hWait = waitbar(0, 'Calculation regularization parameter (L-curve)');
        rho =[]; eta = [];
        regvalC = zeros(nSamp, 1);
        for i=1:nSamp
            %%waitbar(i/nSamp, hWait);
            [rho(i,:), eta(i,:), regval(i,:)]= l_curve(obj.matU,obj.vectS,vectB(:, i), 'tikh');
            [regvalC(i),rho_c,eta_c] = l_corner(rho(i,:),eta(i,:),regval(i,:),obj.matU,obj.vectS,vectB(:, i), 'tikh');
        end
        %%close(hWait);
        avrho = mean(rho, 1); aveta = mean(eta, 1); avreg = mean(regval, 1);
        %hfig = figure('Name', 'Average L-curve');
        %plot_lc(avrho,aveta,'-',1,avreg);
        %plot_lc(rho(round(nSamp/2),:), eta(round(nSamp/2),:),'-',1, regval(round(nSamp/2),:));

        lambda = mean(regvalC);        
       % disp(['Calculated Lambda using Lcurve: ' num2str(lambda)]);
       % disp(['StdDev: ' num2str(std(regvalC))]);
        lambda = lambda * ones(nSamp, 1);

    % CRESO method
    elseif strcmp(reg, 'creso')
        cres_lambda = zeros(nSamp, 1);
        for i=1:nSamp
            cres_lambda(i) = creso(obj.matU, obj.vectS, vectB(:, i));
        end
        % Average the lambda parameters found
        lambda = mean(cres_lambda);
        
        % The implemented tikhonov algorithm uses lambda? in it's
        % formula
        lambda = sqrt(lambda);
        
       % disp(['Calculated Lambda using CRESO: ' num2str(lambda)]);
       % disp(['StdDev: ' num2str(std(cres_lambda))]);
        lambda = lambda * ones(nSamp, 1);
    
        
        
        
        
    % Error management
    else
        error('Invalid input regularization parameter/method');
    end
    
    % Use SVD formulation of tikh
    vectA = zeros(size(obj.matV, 1), nSamp);
    %keyboard
    for i=1:nSamp, vectA(:, i) = tikhonov(obj.matU, obj.vectS, obj.matV, vectB(:, i), lambda(i)); end
    
else
    error('Please specify a valid regularization method');
end

end


