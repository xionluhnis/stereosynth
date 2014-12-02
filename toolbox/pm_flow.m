function [ uv ] = pm_flow( left, right, options )
%PM_FLOW Summary of this function goes here
%   Detailed explanation goes here
    addpath('bin');
    
    if nargin < 3
        options = struct();
    end
    
    kdisp = ikdisp(left, right, [], options);
    disp = iknnf_top(left, right, kdisp, options);
    
    % stupid voting by patch position
    [y, x] = ndgrid(0:size(disp, 1)-1, 0:size(disp, 2)-1);
    uv = disp(:, :, 1:2) - cat(3, x, y);
    
    % thresholding and displaying
    for N = [10, 25, 50, 100, 200]
        figure;
        uv_thresh = min(N, max(-N, uv));
        flow = flowToColor(uv_thresh);
        imshow(flow);
        if isfield(options, 'flow_file')
            fname = sprintf(options.flow_file, N);
            imwrite(flow, fname);
        end
    end
end

