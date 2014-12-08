function [ ] = cache_flow( basedir )

    % get the images
    images = find_images(basedir);
    
    % compute motion flow
    flow_dir = fullfile('libs', 'flow_code_v2');
    util_dir = fullfile(flow_dir, 'utils');
    addpath(flow_dir, genpath(util_dir));
    
    uv_dir = fullfile(basedir, '.uv');
    if ~exist(uv_dir, 'dir')
        mkdir(uv_dir);
    end
    
    gcp;
    N = length(images);
    parfor i = 1:N
        fname = images{i};
        [~, name, ~] = fileparts(fname);
        uv_file = fullfile(uv_dir, [name '.mat']);
        if exist(uv_file, 'file')
            continue;
        end
        frames = single(im2double(imread(fname)));
        [left, right] = get_frames(frames);
        uv = estimate_flow_interface(left, right);
        % save flow and a representative image
        save_mat(uv_file, 'uv');
        imwrite(flowToColor(uv), fullfile(uv_dir, [name '.png']));
    end
    
    rmpath(flow_dir, genpath(util_dir));
end

function [left, right] = get_frames(img)
    h0 = floor(size(img, 1) * 0.5);
    h1 = ceil(size(img, 1) * 0.5);
    left = img(1:h0, :, :);
    right = img(h1+1:end, :, :);
end

function [] = save_mat(fname, uv)
    save(fname, uv);
end