function [ im_laby ] = rgb2laby( im, lambda )
%RGB2LABY Transform RGB image into normalized CIE La*b*+y image
%
% INPUT
%   - im        image in sRGB space
%   - lambda    normalization constant for the y channel (default=1)
%
% OUTPUT
%   - im_laby   the image in CIE La*b*+y
%
    im_lab = rgb2lab(im);
    if nargin < 2
        lambda = 1;
    else
        assert(lambda > 0, 'Invalid lambda, should be positive!');
    end
    h0 = size(im, 1)-1;
    [y, ~] = ndgrid(((0:h0)/h0)*lambda, 0:size(im, 2)-1);
    im_laby = cat(3, im_lab, y);
end

