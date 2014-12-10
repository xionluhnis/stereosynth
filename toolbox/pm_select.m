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
function group = pm_select( query, images, options )

    gist_dir = options.gist_dir;

    % compute the gists if not already computed
    N = length(images);
    num_pixels = 0;
    fprintf('* Loading %d gists ', N); t = tic;
    parfor i = 1:N
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
    fprintf('in %f sec.\n', toc(t));

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
    fprintf('* Selection '); t = tic;
    group = knnsearch(G, g(:)', 'K', K);
    fprintf('in %f sec.\n', toc(t));
        
end
