% Method that subsamples an image
% Given an r x c image, we blur the image first
% using the filter kernel produced by generating_kernel,
% the subsample the rows and columns by a factor of 2
%   
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function [out] = pyr_reduce(im)
    % Blur the image
    img_blur = imfilter(im, pyr_kernel, 'conv');

    % Subsample
    img_blur_rows = img_blur(1:2:end,:,:);
    out = img_blur_rows(:,1:2:end,:);
    
    % Note: the result might have rows=(rows/2+1) if rows is odd
end
