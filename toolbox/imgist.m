function [ G, params ] = imgist( im )
%IMGIST Summary of this function goes here
%   Detailed explanation goes here

    gist_path = fullfile('libs', 'gist');
    addpath(gist_path);
    
    param.imageSize = [256 256]; % it works also with non-square images
    param.orientationsPerScale = [8 8 8 8];
    param.numberBlocks = 4;
    param.fc_prefilt = 4;

    % Computing gist requires 1) prefilter image, 2) filter image and collect
    % output energies
    [G, params] = LMgist(im, '', param);

    rmpath(gist_path);
end

