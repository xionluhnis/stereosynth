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
    params.basedir = fullfile('results', 'web');
    params.imagedir = fullfile(params.basedir, 'images');
    params.images = {};
    params.pyramid_type = 'gaussian'; % or 'laplacian'
    params.last_level = 0;
    params.threads = 4;
    params.done = 0;
    
    % arguments
    if length(varargin) == 1
        if ischar(varargin{1})
            params.basedir = varargin{1};
            if ends_with(params.basedir, '.mat')
                new_params = load_mat(params.basedir, []);
                [p, ~, ~] = fileparts(params.basedir);
                params = merge_opt(params, new_params); % merge default with it
                params.basedir = p; % we reset path in case it moved
            end
        elseif isstruct(varargin{1})
            new_params = varargin{1};
            params = merge_opt(params, new_params);
        else
            error('Invalid type for single argument web');
        end
    end
    
    % additional arguments
    if nargin > 1
        for i = 1:2:nargin
            params.(varargin{i}) = varargin{i+1};
        end
    end
    
    % init web
    mkdir(params.basedir);
    
    % list images
    files = dir(params.imagedir);
    new_images = {};
    for i = 1:length(files)
        if files{i}.isdir
            continue;
        end
        fname = files{i}.name;
        if ends_with(lower(files{i}), '.png') ...
        || ends_with(lower(files{i}), '.jpg')
            if find(strcmp(params.images, fname))
                continue; % already loaded
            end
            % new image!
            new_images{end+1} = fname;
        end
    end
    new_images = sort(new_images); % sort new images!
    
    % setup pyramids
    
    
    % process web until done
    while ~params.done
        
    end
end

function opt = merge_opt(o, user_opt)
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
