% Create a Gaussian pyramid
% Essentially this uses reduce for a total of levels times.
% Each image in one scale is half the size of the previous scale
% First scale is simply the original image
%   
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function [gaussout] = gaussian_pyr(im, levels)
    if nargin < 2
        levels = pyr_levels(im);
    end
    gaussout = cell(1,levels+1);
    gaussout{levels+1} = im;
    subsample = im;
    for i = levels:-1:1
        subsample = pyr_reduce(subsample);
        gaussout{i} = subsample;
    end
end