function knnf = pm_query( query, images, options )
%PM_QUERY Compute a knnf query on a set of images

    if nargin < 3
        options.patch_size = 7;
        options.iterations = 6;
    end

    if ischar(images)
        gist_dir = fullfile(images, '.gist');
        images = find_images(images);
    elseif iscell(images)
        gist_dir = tempname;
    else
        error('Unsupported image database');
    end
    if ~exist(gist_dir, 'dir')
        mkdir(gist_dir);
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
            img = im2double(imread(img));
            img = img(1:end/2, :, :);
        else
            assert(isnumeric(img), 'Invalid image type');
            gist_file = [tempname(gist_dir) '.mat'];
            if ~isfloat(img)
                img = im2double(img);
            end
        end
        [h, w, ~] = size(img);
        num_pixels = num_pixels + h * w;
        if exist(gist_file, 'file')
            g = load_mat(gist_file);
        else
            g = imgist(img);
            % and store it
            save(gist_file, 'g');
        end
        G(i, :) = g(:);
    end
    
    % query gist
    g = imgist(query);
    
    % select best images to compute k-nnf with
    if ~isfield(options, 'target_number')
        options.target_number = ceil(...
            get_option(options, 'memory', 50e6) ...
                           / (8 * num_pixels / N)); % 4d image + 4d nnf
    end
    K = min(N, max(get_option(options, 'min_targets', 5), options.target_number));
    group = knnsearch(G, g(:)', 'K', K);
    % sort targets to nothave correlation in indices
    group = group(randperm(K));
    
    % load targets
    targets = cell(K, 1);
    for k = 1:K
        idx = group(k);
        img = images{idx};
        if ischar(img)
            img = single(im2double(imread(img)));
        elseif ~isfloat(img)
            img = single(im2double(img));
        elseif ~isa(img, 'float')
            img = single(img);
        end
        targets{k} = img;
    end
    
    knnf = ixknnf(query, targets, [], options);
end

function o = get_option(options, fname, def)
	if isfield(options, fname)
		o = options.(fname);
	else
		o = def;
	end
end