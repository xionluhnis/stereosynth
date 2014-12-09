
addpath toolbox

% load image
images = find_images('results/1205/web/test');
out_dir = 'results/1208/all';
params = { ...
  struct('pyr_type', 'gaussian'), ...
  struct('pyr_type', 'laplacian'), ...
  struct('pyr_type', 'gaussian', 'transfer_type', 'diff'), ...
  struct('pyr_type', 'laplacian', 'transfer_type', 'diff'), ...
  struct('pyr_type', 'gaussian', 'transfer_type', 'uv'), ...
  struct('pyr_type', 'laplacian', 'transfer_type', 'uv')
};
for i = 1:length(images)
    file = images{i};
    [~, name, ~] = fileparts(file);
    fprintf('Processing %s (%d of %d)\n', name, i, length(images));
    frames = single(im2double(imread(file)));
    [left, right] = get_frames(frames);
    query = imresize(left, 0.25);
    
    for p = 1:length(params)
        result_dir = fullfile(out_dir, name);
        result_file = fullfile(result_dir, ['res' num2str(p) '.mat']);
        if exist(result_file, 'file')
            continue
        end
        options = params{p};
        options.precomputed_uv = 'results/web/images/.uv';
        [synth, data] = stereo_synth(query, 'results/web/images', options);
        if ~exist(result_dir, 'dir')
            mkdir(result_dir);
        end
        save(result_file);
        try
           imwrite(query, fullfile(result_dir, 'left.jpg'));
           imwrite(synth, fullfile(result_dir, ['right-' num2str(p) '.png']));
        catch e
        end
    end
end