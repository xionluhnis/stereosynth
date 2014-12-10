%PM_QUERY Compute a knnf query on a set of images
%
% INPUT
%   - query     the image query
%   - images    either the image directory or a list of images (str or img)
%   - options   query options
%
% OUTPUT
%   - knnf      the resulting knnf
%   - data      struct with fields:
%                   - left: the left images
%                   - right: the right images
%                   - files: the image files (if available)
%
function [knnf, data] = pm_query( query, images, options )

    if nargin < 3
        options.patch_size = 7;
        options.iterations = 6;
    end
    leftright = ~get_option(options, 'only_left', 0);
    % by default, no files
    files = {};

    % get image list at least
    if ischar(images)
        gist_dir = fullfile(images, '.gist');
        images = find_images(images);
        files = images; % we have the files!
    elseif iscell(images)
        gist_dir = tempname;
        if isempty(images)
            error('Empty images database');
        end
        if ischar(images{1})
            files = images; % we also have the files!
        elseif isnumeric(images{1})
            
        end
    else
        error('Unsupported image database');
    end
    
    % replacing data from options
    if isfield(options, 'gist_dir')
        gist_dir = options.gist_dir;
    end
    if isfield(options, 'files')
        files = options.files;
    end
    
    % group selection
    if isfield(options, 'group')
        % provided for us, nothing to do
        K = length(options.group);
        if isfield(options, 'perm')
            data.perm = options.perm;
        else
            data.perm = 1:K;
        end
        data.group = options.group;
        if isfield(options, 'rank')
            data.rank = options.rank;
        else
            data.rank(data.perm) = 1:K;
        end
    else
        
        % selecting the group of images to use for patch search
        options.gist_dir = gist_dir;
        group = pm_select(query, images, options);
        K = length(group);

        % sort targets to nothave correlation in indices
        data.perm = randperm(K);
        data.group = group(data.perm); % permuted order
        data.rank(data.perm) = 1:K; % reverse permutation
    end
    
    % load targets
    data.left = cell(K, 1);
    if leftright
        data.right = cell(K, 1);
    end
    if ~isempty(files)
        data.files = cell(K, 1);
    end
    for k = 1:K
        idx = data.group(k);
        img = images{idx};
        if ischar(img)
            img = single(load_img(img));
        elseif ~isfloat(img)
            img = single(im2double(img));
        elseif ~isa(img, 'float')
            img = single(img);
        end
        
        if leftright
            [left, right] = get_frames(img);
            data.left{k} = left;
            data.right{k} = right;
        else
            data.left{k} = img; % because onlyleft
        end
        if ~isempty(files)
            data.files{k} = files{idx};
        end
    end
    
    % k-nnf computation from query to best set
    t = tic;
    start_nnf = get_option(options, 'start_nnf', []);
    knnf = ixknnf(query, data.left, start_nnf, options);
    fprintf('* k-NNF computed in %f sec.\n', toc(t));
end
