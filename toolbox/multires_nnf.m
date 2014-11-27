function [ nnf ] = multires_nnf( A, B, options, method )
    mexpath = fullfile(pwd, '..', 'bin');
    addpath(mexpath);
    
    % default arguments
    if nargin < 3
        options.method = @isnnf;
    end
    if nargin < 4
        method = param(options, 'method', @isnnf);
    end

    min_scale = param(options, 'min_scale', 35);
    num_scales = param(options, 'num_scales', ...
        count_levels(max([ size(A, 1), size(A, 2), size(B, 1), size(B, 2) ])), ...
        min_scale ...
    );

    % pyramid computation
    pyramid = cell(num_scales, 2);
    pyramid{num_scales, 1} = A;
    pyramid{num_scales, 2} = B;
    for lvl = num_scales-1:-1:1
        pyramid{lvl, 1} = impyramid(pyramid{lvl + 1, 1}, 'reduce');
        pyramid{lvl, 2} = impyramid(pyramid{lvl + 1, 2}, 'reduce');
    end
    
    % multi-resolution nnf
    nnf = []; % start with empty nnf
    iterations = param(options, 'iterations', 6);
    for s = 1:num_scales
        
        % upscaling the nnf when necessary
        if s > 1
            % TODO rescale with padding (to enforce correct values)
        end
        
        % per scale parameters
        opts = options;
        if numel(iterations) > 1
            opts.iterations = iterations(s);
        elseif isfield(options, 'iter_decr')
            iterations = iteration - options.iterations_decr;
            opts.iterations = iterations;
        elseif isfield(options, 'iter_decr_factor')
            iterations = ceil(iteration / options.iter_decr_factor);
            opts.iterations = iterations;
        end
        if isfield(options, 'iter_min')
            opts.iterations = min(options.iter_min, opts.iterations);
        end
        
        % compute new nnf
        nnf = method(A, B, nnf, opts);
    end
end

function levels = count_levels(sze, min_sze)
    levels = 0;
    while sze > min_sze
        levels = levels + 1;
        sze = sze * 0.5;
    end
end

function x = param(options, name, defval)
    if isfield(options, name)
        x = options.(name);
    else
        x = defval;
    end
end