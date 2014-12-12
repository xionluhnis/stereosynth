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
    % load options (and default)
    options.patch_size = 7;
    options.iterations = 6;
    options.transfer_type = 'patch'; % or 'diff' or 'disparity'
    options.vote_filter = fspecial('gaussian', [7, 7], 1);
    if length(varargin) == 1
        user_opt = varargin{1};
        assert(isstruct(user_opt), 'Single argument must be an option struct!');
        % transfer user options
        field_names = fieldnames(user_opt);
        for f=1:length(field_names)
            options.(field_names{f}) = user_opt.(field_names{f});
        end
    elseif length(varargin) > 1
        assert(mod(length(varargin), 2) == 0, 'Multiple arguments must be even!');
        for i = 1:2:length(varargin)
            options.(varargin{i}) = varargin{i+1};
        end
    end
    pyr_type = get_option(options, 'pyr_type', 'laplacian');
    pyr_depth = get_option(options, 'pyr_levels', []);
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
    fprintf('* Building database pyramids '); t = tic;
    parfor i=1:N
        fname = images{i};
        [~, name, ~] = fileparts(fname);
        % check if not alraedy computed
        pyr_file = get_pyr_file(cache_dir, 1, name, pyr_type);
        if exist(pyr_file, 'file')
            continue
        end
        img = im2double(imread(fname));
        left = get_frames(img);
        pyr = get_pyramid(left, pyr_type, pyr_depth);
        levels = length(pyr);
        % cache pyramid levels
        for l=1:levels
            fname = get_pyr_file(cache_dir, l, name, pyr_type);
            save_mat(pyr{l}, fname);
        end
    end
    fprintf('in %f sec.\n', toc(t));
    pyr = get_pyramid(query, pyr_type, pyr_depth);
    
    %% 1 = over the scales (from 1 to N), query with PM
    L = length(pyr);
    pyr_result = cell(1, L);
    transfer_type = get_option(options, 'transfer_type', 'patch');
    l = 1;
    while l <= L
        fprintf('-----------------------------------------\n');
        fprintf('--- PM Query at scale %d of %d\n', l, L);
        fprintf('-----------------------------------------\n');
        
        % potentially upsample knnf from the past level
        % /!\ FIXME this only works if the same exemplars are in memory in the
        % upper level. We should constrain it! (or have all exemplars in
        % memory, but that's a bit unreasonable in real life)
        if get_option(options, 'incremental', 1)
            if l > 1
                prevNNF = data(l-1).knnf;
                assert(check_all(prevNNF(:, :, 3:4:end) < length(data(l-1).group)), 'Indices were corrupted');
                [h, w, ~] = size(pyr{l});
                P = get_option(options, 'patch_size');
                % TODO use a correct resizing (pad and then unpad) like images
                % x+y channels
                newNNF = imresize(prevNNF, [h, w] - P + 1, 'nearest');
                newNNF(:, :, 1:4:end) = newNNF(:, :, 1:4:end) * 2; % x
                newNNF(:, :, 2:4:end) = newNNF(:, :, 2:4:end) * 2; % y
                assert(check_all(newNNF(:, :, 3:4:end) < length(data(l-1).group)), 'Indices got corrupted');
                assert(check_all(newNNF(:, :, 4:4:end) >= 0), 'Distances got corrupted');
                if l == L
                    % TODO compact group at last level by keeping only indices
                     %      that have found a good value in the last level
                    past_groups = unique_set(data(l-1).knnf(:, :, 3:4:end));
                    if length(past_groups) < length(options.group)
                        fprintf('* Keeping %d images of %d\n', length(past_groups), length(options.group));
                        % remap group and NNF indices
                        idx_map(past_groups+1) = (1:length(past_groups))-1; % for C++ 0-based indexing
                        newNNF(:, :, 3:4:end) = bsxfun(@(x, z)idx_map(x+1), newNNF(:, :, 3:4:end), 0);
                        options.group = options.group(past_groups+1);
                    end
                else
                    options.perm = data(l-1).perm;
                    options.group = data(l-1).group;
                    options.rank = data(l-1).rank;
                end
                options.start_nnf = newNNF;
                % we must update the distance that has certainly changed
                options.update_dist = 1;
            else
                % let's get the group from the top level since we want that
                % level
                pyr_images = get_pyr_images(cache_dir, L, images, pyr_type);
                options.gist_dir = fullfile(cache_dir, 'gist', num2str(L));
                group = pm_select(pyr{L}, pyr_images, options);
                options.group = group;
            end
        elseif strcmp(pyr_type, 'gaussian')
            l = L; % no need to do the preliminary levels here
            fprintf('* Jump to last level\n');
        end
        
        %% 2 = query with PM
        options.gist_dir = fullfile(cache_dir, 'gist', num2str(l));
        options.only_left = 1;
        pyr_images = get_pyr_images(cache_dir, l, images, pyr_type);
        
        % get knnf + pyramid data
        t = tic;
        [knnf, pyr_data] = pm_query(pyr{l}, pyr_images, options);
        fprintf('* Query done in %f sec.\n', toc(t));
        
        % extract best of knnf
        t = tic;
        nnf = ixknnf_top(pyr{l}, pyr_data.left, knnf, options);
        fprintf('* k-NNF to NNF in %f sec.\n', toc(t));
        
        % store pyramid data
        pyr_data.query = pyr{l};
        pyr_data.knnf = knnf;
        pyr_data.nnf = nnf;
        pyr_data.right = get_right_images(...
            cache_dir, l, images(pyr_data.group), pyr_type, pyr_depth ...
        );
        assert(all(size(pyr_data.left{1}) == size(pyr_data.right{1})), ...
            'The left and right data have different sizes');
        
        %% 3 = transfer
        t = tic;
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
                if isfield(options, 'precomputed_uv')
                    uvs = fast_uv_data(cache_dir, l, images(pyr_data.group), options.precomputed_uv);
                else
                    uvs = get_uv_data(cache_dir, l, images(pyr_data.group), pyr_type, pyr_depth);
                end
                assert(all(size(uvs{1}(:, :, 1)) == size(pyr_data.right{1}(:, :, 1))), 'UV map has invalid size');
                uv = ixvote(left(:, :, 1:2), uvs, nnf, options);
                right = warpFLColor(left, left, uv(:, :, 1), uv(:, :, 2)); % direction?
                pyr_data.uv = uv;
            otherwise
                error('Unsupported transfer type: %s', transfer_type);
        end
        fprintf('* Transfer in %f sec.\n', toc(t));
        % store this level's result
        pyr_result{l} = right;
        pyr_data.result = right;
        data(l) = pyr_data;
        
        % next level
        l = l + 1;
    end
    
    % collapse pyramid
    switch pyr_type
        case 'laplacian'
            result = pyr_collapse(pyr_result);
        otherwise
            result = pyr_result{L}; % last result by default
    end
    rmpath('bin');
end

%% Helper functions
function b = check_all(mask)
    b = all(mask(:));
end
function pyr = get_pyramid(img, pyr_type, pyr_depth)
    if nargin < 3 || isempty(pyr_depth)
        pyr_depth = pyr_levels(img);
    end
    G = gaussian_pyr(img, pyr_depth);
    switch pyr_type
        case 'gaussian'
            pyr = G;
        case 'laplacian'
            pyr = laplacian_pyr(G);
        otherwise
            error('Unsupported type of pyramid: %s', pyr_type);
    end
end

function file = get_pyr_file(cache_dir, level, name, pyr_type)
    switch pyr_type
        case 'gaussian'
            pyr_dir = 'gpyr';
        case 'laplacian'
            pyr_dir = 'lpyr';
        otherwise
            error('Unsupported pyramid type: %s', pyr_type);
    end
    file = fullfile(cache_dir, pyr_dir, num2str(level), [name '.mat']);
end

function pyr_images = get_pyr_images(cache_dir, level, images, pyr_type)
    N = length(images);
    pyr_images = cell(1, N);
    for i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_images{i} = get_pyr_file(cache_dir, level, name, pyr_type);
    end
end

function rights = get_right_images(cache_dir, level, images, pyr_type, pyr_depth)
    N = length(images);
    rights = cell(1, N);
    right_cache = fullfile(cache_dir, 'right');
    fprintf('* Retrieving right image pyramids '); t = tic;
    parfor i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_file = get_pyr_file(right_cache, level, name, pyr_type);
        if exist(pyr_file, 'file')
            % simply load it
            rights{i} = single(load_mat(pyr_file));
        else
            % we compute the pyramid and cache it
            % - get right frame
            frames = im2double(imread(images{i}));
            [~, right] = get_frames(frames);
            % - get right pyramid
            pyr = get_pyramid(right, pyr_type, pyr_depth);
            for l=1:length(pyr)
                pyr_file = get_pyr_file(right_cache, l, name, pyr_type);
                save_mat(pyr{l}, pyr_file);
            end
            % finally we have the right pyramid slice
            rights{i} = single(pyr{level});
        end
    end
    fprintf('in %f sec.\n', toc(t));
end

function diffs = get_diffs(lefts, rights)
    K = length(lefts);
    assert(length(rights) == K, 'Left and rights with different lenghts');
    diffs = cell(1, K);
    for i = 1:K
        diffs{i} = rights{i} - lefts{i};
    end
end

function uv = get_uv_data(cache_dir, level, images, pyr_type, pyr_depth)
    N = length(images);
    uv = cell(1, N);
    uv_cache = fullfile(cache_dir, 'uv');
    flow_dir = fullfile('libs', 'flow_code_v2');
    util_dir = fullfile(flow_dir, 'utils');
    addpath(flow_dir, genpath(util_dir));
    fprintf('* Retrieving flow fields (this may be long)...\n'); t = tic;
    parfor i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_file = get_pyr_file(uv_cache, level, name, pyr_type);
        if exist(pyr_file, 'file')
            % simply load it
            uv{i} = load_mat(pyr_file);
        else
            % we compute the pyramid and cache it
            % - get right frame
            frames = im2double(imread(images{i}));
            [~, right] = get_frames(frames);
            % - get right pyramid
            pyr = get_pyramid(right, pyr_type, pyr_depth);
            for l=1:length(pyr)
                left_file = get_pyr_file(cache_dir, l, name, pyr_type);
                left = load_mat(left_file);
                right = pyr{l};
                % compute uv map
                flow = estimate_flow_interface(right, left);
                pyr{l} = single(flow); % we replace the pyramid with the flow
                pyr_file = get_pyr_file(uv_cache, l, name, 'gaussian');
                save_mat(pyr{l}, pyr_file);
            end
            % finally we have the right pyramid slice
            uv{i} = pyr{level};
        end
    end
    fprintf('* Flows retrieved in %f sec.\n', toc(t));
end

function uv = fast_uv_data(cache_dir, level, images, precomp_dir)
    N = length(images);
    uv = cell(1, N);
    uv_cache = fullfile(cache_dir, 'uv');
    fprintf('* Retrieving precomputed flow fields\n'); t = tic;
    for i = 1:N
        [~, name, ~] = fileparts(images{i});
        pyr_file = get_pyr_file(uv_cache, level, name, 'gaussian');
        if exist(pyr_file, 'file')
            % simply load it
            uv{i} = load_mat(pyr_file);
        else
            % we load the precomputed full-resolution flow
            flow = load_mat(fullfile(precomp_dir, [name '.mat']));
            % and compute a pyramid for it (with semantic scaling)
            pyr = get_pyramid(flow, 'gaussian');
            for l=1:length(pyr)
                depth = length(pyr) - l;
                factor = 2^depth;
                pyr{l} = single(pyr{l} / factor);
                pyr_file = get_pyr_file(uv_cache, l, name, 'gaussian');
                save_mat(pyr{l}, pyr_file);
            end
            % finally we have the right pyramid slice
            uv{i} = pyr{level};
        end
    end
    fprintf('* Flows retrieved in %f sec.\n', toc(t));
end

function s = unique_set(x)
    s = unique(x(:));
end