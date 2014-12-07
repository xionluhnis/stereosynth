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
    if ~isfloat(im)
        im = im2double(im);
    end
    if isa(im, 'single')
        im = double(im);
        is_single = 1;
    else
        is_single = 0;
    end
    if nargin < 2
        lambda = 1;
    else
        assert(lambda > 0, 'Invalid lambda, should be positive!');
    end
    cform = makecform('srgb2lab');
    im_lab = applycform(im, cform);
    h0 = size(im, 1)-1;
    [y, ~] = ndgrid(((0:h0)/h0)*lambda, 0:size(im, 2)-1);
    im_laby = cat(3, ... 
        im_lab(:, :, 1) / 100, ...          % L in [0;100]
        (80 + im_lab(:, :, 2)) / 160, ...   % a in [-80;+80]
        (80 + im_lab(:, :, 3)) / 160, ...   % b in [-80;+80]
         y ...
    );
    if is_single
        im_laby = single(im_laby);
    end
end

