function [ uv ] = pm_flow( left, right, options )
%PM_FLOW Summary of this function goes here
%   Detailed explanation goes here
    addpath('bin');
    
    if nargin < 3
        options = struct();
    end
    if ~isfield(options, 'flow_type')
        options.flow_type = 'flow';
        options.use_xy = true;
    end
    switch options.flow_type
        case {'i', 'int', 'integer'}
            uv = int_flow(left, right, options);
        case {'f', 'float'}
            uv = float_flow(left, right, options);
        case {'flow', 'kflow'}
        	uv = fkdisp_flow(left, right, options);
        	return % adaptive thresholding
        otherwise
            error('Invalid flow type: %s', options.flow_type);
    end
    
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

function uv = int_flow(left, right, options)
    kdisp = ikdisp(left, right, [], options);
    disp = iknnf_top(left, right, kdisp, options);
    
    % stupid voting by patch position
    [y, x] = ndgrid(0:size(disp, 1)-1, 0:size(disp, 2)-1);
    uv = disp(:, :, 1:2) - cat(3, x, y);
end

function uv = float_flow(left, right, options)
    kdisp = fkdisp(left, right, [], options);
    disp = fknnf_top(left, right, kdisp, options);
    
    % stupid voting by patch position
    [y, x] = ndgrid(0:size(disp, 1)-1, 0:size(disp, 2)-1);
    uv = disp(:, :, 1:2) - cat(3, x, y);
end

function uv = fkdisp_flow(left, right, options);
	if ~isfield(options, 'patch_size')
		options.patch_size = 7;
	end
	if ~isfield(options, 'vote_filter')
		options.vote_filter = fspecial('gaussian', options.patch_size * [1, 1], 1); % short gaussian mask
	end
	if isfield(options, 'use_xy') && options.use_xy
		cw = get_option(options, 'color_weight', size(left, 1));
		[y, x] = ndgrid(1:size(left, 1), 1:size(right, 2)); % add x and y to enforce close disparity patches
		left = cat(3, cw * left, x, y);
		right = cat(3, cw * right, x, y);
	end
	kdisp = fkdisp(left, right, [], options); % get knnf
	uv = fkdispvote(left, right, kdisp, options); % vote disparity
end

function o = get_option(options, fname, def)
	if isfield(options, fname)
		o = options.(fname);
	else
		o = def;
	end
end