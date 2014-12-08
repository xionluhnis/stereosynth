%STEREO_SYNTH Synthesize a stereo frame from a query using many images
%
% INPUT
%   - query     the image query
%   - images    either the image directory or a list of images (str or img)
%   - options   query options
%
% OUTPUT
%   - result    the synthesized frame
%   - data      additional data linked to the result
%
function [result, data] = stereo_synth( query, images, options )
    if nargin < 3
        options.patch_size = 7;
        options.iterations = 6;
        options.pyramid_type = 'laplacian'; % or 'gaussian'
        options.transfer_type = 'patch'; % or 'diff' or 'disparity'
    end
    files = {};
    
    %% get image list and set base caching directory
    if ischar(images)
        cache_dir = fullfile(images, '.cache');
        images = find_images(images);
        files = images; % we have the files!
    elseif iscell(images)
        assert(~isempty(images), 'Empty image database');
        assert(isnumeric(images{1}), 'Invalid non-numeric cells');
        cache_dir = tempname;
    else
        error('Unsupported image database');
    end
    if isfield(options, 'cache_dir')
        cache_dir = options.cache_dir;
    end
    if ~exist(cache_dir, 'dir')
        mkdir(cache_dir);
    end
    options.files = files;
    
    %% build left frame pyramid
    N = length(images);
    for i=1:N
        img = get_image(images, i);
        left = get_frames(img);
        pyr = get_pyramid(left, options);
        levels = length(pyr);
        % cache pyramid levels
        for l=1:levels
            left = pyr{l};
            fname = get_pyr_file(cache_dir, l, i);
            save(fname, 'left');
        end
    end
    pyr = get_pyramid(query, options);
    
    %% 1 = over the scales (from 1 to N), query with PM
    L = length(pyr);
    pyr_result = cell(1, L);
    transfer_type = get_option(options, 'transfer_type', 'patch');
    for l = 1:L
        
        %% 2 = query with PM
        options.gist_dir = fullfile(cache_dir, 'gist', num2str(l));
        options.only_left = 1;
        pyr_images = get_pyr_images(cache_dir, l, N);
        
        % get knnf + pyramid data
        [knnf, pyr_data] = pm_query(pyr{l}, pyr_images, options);
        
        % extract best of knnf
        nnf = ixknnf_top(pyr{l}, pyr_data.left, knnf, options);
        
        % store pyramid data
        data(l).knnf = knnf;
        data(l).nnf = nnf;
        data(l).data = pyr_data;
        data(l).right = get_right_images(cache_dir, l, pyr_data);
        
        %% 3 = transfer
        left = pyr{l};
        switch transfer_type
            case 'patch'
                right = ixvote(left, pyr_data.right, nnf, options);
            case 'diff'
                rights = get_right_images(cache_dir, l, pyr_data);
                diffs = get_diffs(pyr_data.left, rights);
                diff = ixvote(left, diffs, nnf, options);
                right = left + diff;
                data(l).diff = diff;
            case 'uv'
                uvs = get_uv_data(cache_dir, l, pyr_data);
                uv = ixvote(left, uvs, nnf, options); % DEBUG this, it doesn't work well...
                right = warpFLColor(left, left, -uv(:, :, 1), -uv(:, :, 2));
                data(l).uv = uv;
            otherwise
                error('Unsupported transfer type: %s', transfer_type);
        end
        % store this level's result
        pyr_result{l} = right;
        data(l).result = right;
    end
    
    % collapse pyramid
    switch options.pyramid_type
        case 'laplacian'
            result = pyr_collapse(pyr_results);
        otherwise
            result = pyr_result{L}; % last result by default
    end
end

%% Helper functions
function img = get_image(images, i)
    img = images{i};
    if ischar(img)
        img = im2double(imread(img));
    elseif ~isfloat(img)
        img = im2double(img);
    end
end

function pyr = get_pyramid(img, options)
    levels = get_option(options, 'pyr_levels', pyr_levels(img));
    G = gaussian_pyr(img, levels);
    switch options.pyramid_type
        case 'gaussian'
            pyr = G;
        case 'laplacian'
            pyr = laplacian_pyr(G);
        otherwise
            error('Unsupported type of pyramid: %s', options.pyramid_type);
    end
end

function file = get_pyr_file(cache_dir, level, idx)
    file = fullfile(cache_dir, 'pyr', num2str(level), [num2str(idx) '.mat']);
end

function images = get_pyr_images(cache_dir, level, N)
    images = cell(1, N);
    for i = 1:N
        images{i} = get_pyr_file(cache_dir, level, i);
    end
end

function right = get_right_images(cache_dir, level)
    % TODO Implement right frame retrieval with potential caching too
    error('Yet to be implemented');
end

function diffs = get_diffs(lefts, rights)
    K = length(lefts);
    assert(length(right) == K, 'Left and rights with different lenghts');
    diffs = cell(1, K);
    for i = 1:K
        diffs{i} = rights{i} - lefts{i};
    end
end

function uv = get_uv_data(cache_dir, level)
    % TODO Implement uv retrieval with potential caching too
    error('Yet to be implemented');
end