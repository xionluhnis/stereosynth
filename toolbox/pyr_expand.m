% Function that takes an r x c image, and creates an expanded version
% of size 2r x 2c.  The r x c image is interleaved in checkerboard style
% and then blurred using the generating kernel
%   
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function [expout] = pyr_expand(im, dim)
    % Allocate memory for expansion
    [rows, cols, num_ch] = size(im);
    expout = zeros([2*rows 2*cols num_ch]);
    expout(1:2:2*rows, 1:2:2*cols, :) = im;

    % Smooth the image
    expout = imfilter(expout, pyr_kernel, 'conv');
    
    % Must scale by 4 to ensure that net weight contributing
    % to each output pixel is 1.
    expout = 4*expout;
    
    if nargin >= 2
        expout = expout(1:dim(1),1:dim(2),:);
    end
end
