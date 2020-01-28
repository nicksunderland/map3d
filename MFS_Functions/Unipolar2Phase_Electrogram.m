  function [Electrogram_Phase] = Unipolar2Phase_Electrogram(Electrogram, Period)
        % MATLAB function calculating phase of unipolar electroram
        %   Parameters:
        %       Electrogram - original electrogram, should be one-dimensional array
        %                     of doubles
        %       Period - base cycle length of the activity (can be obtained as dominant frequency of 
        %               electrogram or just put manually after electrogram inspection;
        %               units: same as sampling of Electrogram);
        %                
        % STEP ONE: SINUSOIDAL RECOMPOSITION
        Recomposed_Signal = zeros(numel(Electrogram),1);
        Period = round(Period); 

        % create sinusoid (to speed up calculations)
        Sinusoid_Wavelet = zeros(Period+1,1);
        for t=1:Period
            Sinusoid_Wavelet(t) = sin( 2*pi*(t-Period/2)/Period);
        end

        % calculation of the recomposed signal
        for t=2:numel(Recomposed_Signal)-1
            diff = Electrogram(t+1) - Electrogram(t-1);
            if diff < 0 
                for tt=-floor(Period/2):floor(Period/2)
                    if t+tt>0 && t+tt<numel(Electrogram)
                       Recomposed_Signal(t+tt)= Recomposed_Signal(t+tt) + diff*Sinusoid_Wavelet(floor(tt+Period/2+1)); 
                    end
                end
            end
        end

        % STEP TWO: COMPUTATION OF THE PHASE
        h = hilbert(Recomposed_Signal);
        Electrogram_Phase=atan2(real(h),-imag(h));
        Phase = h; 
    end