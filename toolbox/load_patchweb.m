%LOAD_PATCHWEB - load a patch web workspace
%
% SYNOPSIS
% 
%   ws = load_patchweb();
%   ws = load_patchweb(params);
%   ws = load_patchweb(path);
%   ws = load_patchweb(..., p, val);
%
% INPUT
%   - params    the parameter of the workspace
%   - basedir   the path to the workspace base with default parameters
%               or the path to the workspace .mat file containing settings
%   - p, val    pairs of parameters (string, value)
%
% OUTPUT
%   - ws    the workspace data that can be used for search with patchweb
%
function ws = load_patchweb( varargin )
    % default parameters
    ws.basedir = fullfile('results', 'web');
    ws.imagedir = fullfile(ws.basedir, 'images');
    ws.images = {};
    ws.pyramid_type = 'gaussian'; % or 'laplacian'
    ws.pyramid.G = {};
    ws.pyramid.L = {};
    ws.pyr_level = 1;
    ws.threads = 4;
    ws.done = 0;
    
    % arguments
    if length(varargin) == 1
        if ischar(varargin{1})
            ws.basedir = varargin{1};
            if ends_with(ws.basedir, '.mat')
                new_params = load_mat(ws.basedir);
                [p, ~, ~] = fileparts(ws.basedir);
                ws = merge_params(ws, new_params); % merge default with it
                ws.basedir = p; % we reset path in case it moved
            end
        elseif isstruct(varargin{1})
            new_params = varargin{1};
            ws = merge_params(ws, new_params);
        else
            error('Invalid type for single argument web');
        end
    elseif exist(fullfile(ws.basedir, 'web.mat'), 'file')
        p = ws.basedir;
        ws = merge_params(ws, load_mat(fullfile(ws.basedir, 'web.mat')));
        ws.basedir = p;
    end
    
    % additional arguments
    if nargin > 1
        for i = 1:2:nargin
            ws.(varargin{i}) = varargin{i+1};
        end
    end
    
    % init web
    if ~exist(ws.basedir, 'dir')
        mkdir(ws.basedir);
    end
    
    % list images
    files = dir(ws.imagedir);
    new_images = {};
    for i = 1:length(files)
        if files(i).isdir
            continue;
        end
        fname = fullfile(ws.imagedir, files(i).name);
        if ends_with(lower(fname), '.png') ...
        || ends_with(lower(fname), '.jpg')
            if find(strcmp(ws.images, fname))
                continue; % already loaded
            end
            % new image!
            new_images{end+1} = fname;
        end
    end
    new_images = sort(new_images); % sort new images!
    
    % setup pyramids
    idx_prefix = max([0, length(ws.images) - 1]);
    for i = 1:length(new_images) 
        idx = idx_prefix + i;
        
        % load image
        fname = new_images{i};
        fprintf('Caching image #%d (idx=%d) | %s\n', i, idx, fname);
        ws.images{idx} = fname; % store in list
        I = single(im2double(imread(fname)));
        I = rgb2laby(I);
        
        % work on both left and right
        pair_prefix = { 'left', 'right' };
        pair = { I(1:end/2, :, :), I(end/2+1:end, :, :) };
        for p = 1:2
            fprintf('Section %s\n', pair_prefix{p});
            % compute gaussian pyramid anyway
            Gpyr = gaussian_pyr(pair{p});
            cache_dir = fullfile(ws.basedir, 'cache', pair_prefix{p});
            switch ws.pyramid_type
                case 'gaussian'
                    save_pyramid(Gpyr, cache_dir, num2str(idx));
                case 'laplacian'
                    Lpyr = laplacian_pyr(Gpyr);
                    save_pyramid(Lpyr, cache_dir, num2str(idx));
                otherwise
                    error('Unsupported type of pyramid: %s', ws.pyramid_type);
            end
        end
        
        % add infinity as convergence score to start
        ws.score{idx} = inf;
    end 
    
    % release unused memory
    clear I; clear Gpyr; clear Lpyr;
    
    % restart web from base level if we have new images
    if ~isempty(new_images)
        ws.pyr_level = 1;
    end
    
    % save current workspace
    web_file = fullfile(ws.basedir, 'web.mat');
    save(web_file, 'ws');
    
    % parallel pools and group sizes
    threads = ws.threads;
    members = 7; % number of images per group
    if threads > 1
        p = gcp;
        if p.NumWorkers < threads
            parpool(threads);
        end
    end
    
    % running file that stops the computation when removed
    run_file = fullfile(ws.basedir, 'running');
    
    % process web until done
    while ~ws.done && exist(run_file, 'file')
        
        % 1 = create groups
        groups = cell(threads, members);
        
        
        % 2 = compute nnf for the groups, in parallel
        parfor (i = 1:threads, threads)
            group = groups{i, :};
        end
        
        % 3 = merge nnf data
        
        
        % 4 = save to disk
        
    end
end

function opt = merge_params(o, user_opt)
    opt = o;
    if ~isstruct(opt) || ~isstruct(user_opt)
        error('Option argument is not a struct!');
    end
    % overwrite default with these
    user_fields = fieldnames(user_opt);
    for i = 1:numel(user_fields)
        f = user_fields{i};
        opt.(f) = user_opt.(f);
    end
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
