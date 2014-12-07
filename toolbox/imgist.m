function [ G, params ] = imgist( im )
%IMGIST Compute gist descriptor of an image (color images get 3 of them)

    gist_path = fullfile('libs', 'gist');
    addpath(gist_path);
    
    param.imageSize = [256 256]; % it works also with non-square images
    param.orientationsPerScale = [8 8 8 8];
    param.numberBlocks = 4;
    param.fc_prefilt = 4;

    % Computing gist requires 1) prefilter image, 2) filter image and collect
    % output energies
    num_ch = size(im, 3);
    if num_ch == 1
        [G, params] = LMgist(im, '', param);
    else
        [G, params] = LMgist(im(:, :, 1), '', param);
        for ch = 2:num_ch
            G(ch, :) = LMgist(im(:, :, ch), '', param);
        end
    end

    rmpath(gist_path);
end

