classdef signalObject < handle
    % Custom class to manage catheter signals 
    %   SIGNALOBJECT provides methods to manipulate a matrix of signals.
    %   
    % Input: 
    % - signalMatrix with the data, rows are the channels, and column
    % the time samples
    % 
    %Optional inputs:
    % - 'label': label 
    % - 'tri': TriRep geometry associated to the signals
    % - 'samp': sampling rate in HZ (default 3kHz)
    % - 'badLeads': boolean vector with the bad leads
    % 

    properties (Dependent)
        rawSignal;  % Signal
        processedSignal; % Processed Signal
        tri = '';    % TriRep if any

        
    end
    properties (GetAccess = 'public', SetAccess = 'public')
        badLeads;        % if =1, good lead
        sigLabel='';   % channel label
        markers=[];
        markersLabel=[];
        triRepIndex = 0; % assign X index in triRep function
    end
    
    properties (GetAccess = 'public', SetAccess = 'private')
%        tri = '';    % TriRep if any
        nChannel;
        nSamp; % Number of samples
        samp;   % Sampling rate
        filteringDetails = ''; % string that contains the filtering details
    end
    
    properties (Access = 'private')
        pTri;
        pRawSignal;                                   % Raw signal
        pProcessedSignal;            % filteredSignal
        
    end
    
    
    
    methods
        %% Constructor
        function obj = signalObject(signal,varargin)
            
            obj.pRawSignal = signal;
            obj.pProcessedSignal = signal;
            obj.nSamp = size(signal,2);
            obj.nChannel = size(signal,1);
            
            opts = parseArgsToStruct(varargin, ...
                {'label', 'tri','samp','badLeads'}, ...
                {'sigLabel','tri','samp','badLeads'}, ...
                {'',  [], 3000,0 });
            
            fnames = fieldnames(opts);
            for i = 1:length(fnames), obj.(fnames{i}) = opts.(fnames{i}); end
            if obj.badLeads==1,             obj.badLeads = true(1,size(signal,1)); end
            
        end
        
        
        
        %% Reset the data after processing
        function resetProcessedSignal(obj)
            % reset all the processing perfomed and restored the raw data
            obj.pProcessedSignal = obj.pRawSignal;
            obj.filteringDetails = '';
        end 
        
        %% GET/SET 
        function set.tri(obj,tri)
            if ~isempty(tri)
                if isa(tri,'triangulation')
                    obj.pTri = tri;
                else
                    error('tri coordinates not valides');
                end
            end
        end
        function outraw = get.rawSignal(obj)
            outraw = obj.pRawSignal;
        end
        function outraw = get.processedSignal(obj)
            outraw = obj.pProcessedSignal;
        end
        
%         %% Ploting function
%         function plot(obj,channel)
%             % Plot function is used to plot the signal either one by one,
%             % or all surimposed
%             % input:
%             % obj: signal Object
%             % channel: channel to display if  ommitted or empty
%             % displays all channels
%             % 
%             % keypress:
%             % uparrow/downarrow to navigate in the channels
%             % backspace to mark the current channel as badLeads
%             % space/enter to mark the current channel as good leads
%             
%             
%             if nargin==1|| isempty(channel) channel = 1:obj.nChannel;end
%             
%             index = 1:obj.nSamp;
%             
% 
%             % Figure plot
%             scaleBar = median(max(obj.processedSignal')-min(obj.processedSignal'))/10;%1000;%1/10*std(obj.pProcessedSignal(:));
%             plotSig1 = obj.pRawSignal(:,index);
%             plotSig2 = obj.pProcessedSignal(:,index);
%             if size(channel,2)==1
%                 hplot1 = []; hplot2 = [];
%                 hfig = figure('Name', 'Check Filtering - press up/down keys to change channel', ...
%                     'KeypressFcn', {@fkeypress}, 'Units', 'Normalized', 'Position', [0.2 0.3 0.6 0.4]);
%                 hold on
%                 if(obj.badLeads(channel)==true)
%                     hplot1 = plot(plotSig1(channel,:)','b');
%                     hplot2 = plot(plotSig2(channel,:)','color',[0 0.2 0]);
%                     hScaleBar = line([0 0],[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar],'LineWidth',3,'Color','m');
% 
%                     legend('Original','Processed');
%                 else
%                     hplot1 = plot(plotSig1(channel,:)','r');
%                     hplot2 = plot(plotSig2(channel,:)','r');
%                     hScaleBar = line([0 0],[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar],'LineWidth',3,'Color','m');
% 
%                     legend('Bad Channel');
%                 end
%                 title(['Channel #' num2str(channel)] );
%             else
%                 plot(plotSig1(channel,index)','b');
%                 hold on
%                 plot(plotSig1(channel,index)','r');
%                 title([obj.sigLabel] );
%             end
%             
%             hold off
%             % h = warndlg('Scroll through signals using arrows, press backspace to mark as a bad channel (red) and return to mark as a good channel (blue)','Instructions');
%             % waitfor(hfig);
%             
%             % Key presses
%             function fkeypress(hObj,event)
%                 if strcmpi(event.Key, 'downarrow') || strcmpi(event.Key, 'rightarrow')
%                     channel = channel+1;
%                     if(channel ==(size(plotSig1,1)+1)); channel =1; end
%                     if(obj.badLeads(channel)==true)
%                         set(hplot1, 'YData', plotSig1(channel,:)','Color','b');
%                         set(hplot2, 'YData', plotSig2(channel,:)','color',[0 0.2 0]);
%                         set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
% 
%                         legend('Original','Processed');
%                     else
%                         set(hplot1, 'YData', plotSig1(channel,:)','Color','r');
%                         set(hplot2, 'YData', plotSig2(channel,:)','Color','r');
%                          set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
%                        legend('Bad Channel');
%                     end
%                     title(['Channel #' num2str(channel)] );
%                 elseif (strcmpi(event.Key, 'uparrow') || strcmpi(event.Key, 'leftarrow'))
%                     channel = channel-1;
%                     if(channel==0); channel = size(plotSig1,1);end
%                     if(obj.badLeads(channel)==true)
%                         set(hplot1, 'YData', plotSig1(channel,:)','Color','b');
%                         set(hplot2, 'YData', plotSig2(channel,:)','color',[0 0.2 0]);
%                         set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
%                         legend('Original','Processed');
%                     else
%                         set(hplot1, 'YData', plotSig1(channel,:)','Color','r');
%                         set(hplot2, 'YData', plotSig2(channel,:)','Color','r');
%                         set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
%                         legend('Bad Channel');
%                     end
%                     title(['Channel #' num2str(channel)] );
%                 elseif strcmpi(event.Key, 'backspace') || strcmpi(event.Key, 'b')
%                     obj.badLeads(channel) = false;
%                     set(hplot1, 'YData', plotSig1(channel,:)','Color','r');
%                     set(hplot2, 'YData', plotSig2(channel,:)','Color','r');
%                     set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
%                     legend('Bad Channel');
%                 elseif strcmpi(event.Key, 'return') || strcmpi(event.Key, 'space')
%                     obj.badLeads(channel) = true;
%                     set(hplot1, 'YData', plotSig1(channel,:)','Color','b');
%                     set(hplot2, 'YData', plotSig2(channel,:)','color',[0 0.2 0]);
%                     set(hScaleBar,'YData',[mean(plotSig2(channel(1),:))-scaleBar mean(plotSig2(channel(1),:))+scaleBar]);
%                     legend('Original','Processed');
%                 end
%                 
%             end
%             
%             
%         end
        
        
    end
end

