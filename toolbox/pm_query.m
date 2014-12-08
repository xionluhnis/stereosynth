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

    % compute the gists if not already computed
    N = length(images);
    num_pixels = 0;
    for i = 1:N
        fprintf('Loading gist %d of %d\n', i, N);
        img = images{i};
        if ischar(img)
            [~, name, ~] = fileparts(img);
            gist_file = fullfile(gist_dir, [name '.mat']);
            img = load_img(img);
        else
            assert(isnumeric(img), 'Invalid image type');
            gist_file = [tempname(gist_dir) '.mat'];
            if ~isfloat(img)
                img = im2double(img);
            end
            if leftright
                img = get_frames(img);
            end
        end
        img = get_frames(img); % we keep only the left frame for the gist
        [h, w, ~] = size(img);
        num_pixels = num_pixels + h * w;
        if exist(gist_file, 'file')
            g = load_mat(gist_file);
        else
            g = imgist(img);
            % and store it
            save_mat(g, gist_file);
        end
        G(i, :) = g(:);
    end
    
    % query gist
    g = imgist(query);
    
    % choose group size
    if ~isfield(options, 'target_number')
        max_mem = get_option(options, 'memory', 50e6); % in bytes
        avg_pixels = num_pixels / N;
        avg_mem = 8 * avg_pixels; % 4D image + 4D nnf
        options.target_number = ceil( max_mem / avg_mem );
    end
    min_targets = get_option(options, 'min_targets', 5); % minimum in knnf
    K = min( N, max(min_targets, options.target_number) );
    
    % select K best images to compute k-nnf with
    group = knnsearch(G, g(:)', 'K', K);
    
    % clear that memory
    clear G; clear g;
    
    % sort targets to nothave correlation in indices
    group = group(randperm(K));
    data.group = group;
    
    % load targets
    data.left = cell(K, 1);
    if leftright
        data.right = cell(K, 1);
    end
    if ~isempty(files)
        data.files = cell(K, 1);
    end
    for k = 1:K
        idx = group(k);
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
    knnf = ixknnf(query, data.left, [], options);
end

function res = ends_with(str, pat)
    S = length(str);
    P = length(pat);
    if S < P
        res = 0;
    else
        res = strcmp(str(end-P+1:end), pat);
    end
end


function img = load_img(file)
    if ends_with(file, '.mat')
        img = load_mat(file); 
    else
        img = im2double(imread(file));
    end
end