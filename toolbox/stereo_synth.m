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
function [result, data] = stereo_synth( query, images, varargin )
    addpath('bin');
    switch length(varargin)
        case 0
            options.patch_size = 7;
            options.iterations = 6;
            options.pyramid_type = 'laplacian'; % or 'gaussian'
            options.transfer_type = 'patch'; % or 'diff' or 'disparity' 
        case 1
            options = varargin{1};
            assert(isstruct(options), 'Single argument must be an option struct!');
        otherwise
            assert(mod(length(varargin), 2) == 0, 'Multiple arguments must be even!');
            for i = 1:2:length(varargin)
                options.(varargin{i}) = varargin{i+1};
            end
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
        % store temporarily to disk for referencing
        image_dir = fullfile(cache_dir, 'images');
        if ~exist(image_dir, 'dir')
            mkdir(image_dir);
        end
        for i = 1:length(images)
            tmp_name = fullfile(image_dir, [num2str(i) '.png']);
            imwrite(images{i}, tmp_name);
            images{i} = tmp_name;
        end
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
        fname = images{i};
        [~, name, ~] = fileparts(fname);
        % check if not alraedy computed
        fname = get_pyr_file(cache_dir, 1, name);
        if exist(fname, 'file')
            continue
        end
        img = im2double(imread(fname));
        left = get_frames(img);
        pyr = get_pyramid(left, options);
        levels = length(pyr);
        % cache pyramid levels
        for l=1:levels
            fname = get_pyr_file(cache_dir, l, name);
            save_mat(pyr{l}, fname);
        end
    end
    pyr = get_pyramid(query, options);
    
    %% 1 = over the scales (from 1 to N), query with PM
    L = length(pyr);
    pyr_result = cell(1, L);
    transfer_type = get_option(options, 'transfer_type', 'patch');
    for l = 1:L
        fprintf('PM Query at scale %d of %d\n', l, L);
        
        %% 2 = query with PM
        options.gist_dir = fullfile(cache_dir, 'gist', num2str(l));
        options.only_left = 1;
        pyr_images = get_pyr_images(cache_dir, l, images);
        
        % get knnf + pyramid data
        [knnf, pyr_data] = pm_query(pyr{l}, pyr_images, options);
        
        % extract best of knnf
        nnf = ixknnf_top(pyr{l}, pyr_data.left, knnf, options);
        
        % store pyramid data
        pyr_data.knnf = knnf;
        pyr_data.nnf = nnf;
        pyr_data.right = get_right_images(cache_dir, l, images(pyr_data.group), options);
        
        %% 3 = transfer
        left = pyr{l};
        switch transfer_type
            case 'patch'
                right = ixvote(left, pyr_data.right, nnf, options);
            case 'diff'
                diffs = get_diffs(pyr_data.left, pyr_data.right);
                diff = ixvote(left, diffs, nnf, options);
                right = left + diff;
                pyr_data.diff = diff;
            case 'uv'
                uvs = get_uv_data(cache_dir, l, images(pyr_data.group));
                uv = ixvote(left, uvs, nnf, options);
                right = warpFLColor(left, left, -uv(:, :, 1), -uv(:, :, 2)); % direction?
                pyr_data.uv = uv;
            otherwise
                error('Unsupported transfer type: %s', transfer_type);
        end
        % store this level's result
        pyr_result{l} = right;
        pyr_data.result = right;
        data(l) = pyr_data;
    end
    
    % collapse pyramid
    switch options.pyramid_type
        case 'laplacian'
            result = pyr_collapse(pyr_result);
        otherwise
            result = pyr_result{L}; % last result by default
    end
    rmpath('bin');
end

%% Helper functions
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

function file = get_pyr_file(cache_dir, level, name)
    file = fullfile(cache_dir, 'pyr', num2str(level), [name '.mat']);
end

function pyr_images = get_pyr_images(cache_dir, level, images)
    N = length(images);
    pyr_images = cell(1, N);
    for i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_images{i} = get_pyr_file(cache_dir, level, name);
    end
end

function rights = get_right_images(cache_dir, level, images, options)
    N = length(images);
    rights = cell(1, N);
    right_cache = fullfile(cache_dir, 'right');
    for i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_file = get_pyr_file(right_cache, level, name);
        if exist(pyr_file, 'file')
            % simply load it
            rights{i} = load_mat(pyr_file);
        else
            % we compute the pyramid and cache it
            % - get right frame
            frames = im2double(imread(images{i}));
            [~, right] = get_frames(frames);
            % - get right pyramid
            pyr = get_pyramid(right, options);
            for l=1:length(pyr)
                save_mat(pyr{l}, pyr_file);
            end
            % finally we have the right pyramid slice
            rights{i} = pyr{l};
        end
    end
end

function diffs = get_diffs(lefts, rights)
    K = length(lefts);
    assert(length(right) == K, 'Left and rights with different lenghts');
    diffs = cell(1, K);
    for i = 1:K
        diffs{i} = rights{i} - lefts{i};
    end
end

function uv = get_uv_data(cache_dir, level, images, options)
    N = length(images);
    uv = cell(1, N);
    uv_cache = fullfile(cache_dir, 'uv');
    gcp;
    parfor i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_file = get_pyr_file(uv_cache, level, name);
        if exist(pyr_file, 'file')
            % simply load it
            uv{i} = load_mat(pyr_file);
        else
            % we compute the pyramid and cache it
            % - get right frame
            frames = im2double(imread(images{i}));
            [~, right] = get_frames(frames);
            % - get right pyramid
            pyr = get_pyramid(right, options);
            for l=1:length(pyr)
                save_mat(pyr{l}, pyr_file);
            end
            % finally we have the right pyramid slice
            uv{i} = pyr{l};
        end
    end
end