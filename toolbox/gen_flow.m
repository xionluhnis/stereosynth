
addpath bin
addpath toolbox
addpath libs/flow_code_v2/utils/flowColorCode

% load image
images = {'/media/akaspar/BigBertha/akaspar/Data/bbb/frame-0076.jpg', '/media/akaspar/BigBertha/akaspar/Data/bbb/frame-0391.jpg'};
N = length(images);
out_dir = 'results/1212/flow-hd';
if ~exist(out_dir, 'dir')
    mkdir(out_dir);
end
parfor i = 1:N
    file = images{i};
    [~, name, ~] = fileparts(file);
    result_file = fullfile(out_dir, [name '.mat']);
    fprintf('Processing %s (%d of %d)\n', name, i, N);
    if exist(result_file, 'file')
        continue
    end
    frames = single(im2double(imread(file)));
    [left, right] = get_frames(frames);
    uv = pm_flow(right, left);
    
    save_mat(uv, result_file);
    imwrite(flowToColor(uv), fullfile(out_dir, [name '.png']));
end