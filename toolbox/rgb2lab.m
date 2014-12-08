function [ im_lab ] = rgb2lab( im )
%RGB2LABY Transform RGB image into normalized CIE La*b* image
%
% INPUT
%   - im        image in sRGB space
%
% OUTPUT
%   - im_lab   the image in CIE La*b*
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
    cform = makecform('srgb2lab');
    im_lab = applycform(im, cform);
    im_lab = cat(3, ... 
        im_lab(:, :, 1) / 100, ...          % L in [0;100]
        (80 + im_lab(:, :, 2)) / 160, ...   % a in [-80;+80]
        (80 + im_lab(:, :, 3)) / 160  ...   % b in [-80;+80]
    );
    if is_single
        im_lab = single(im_lab);
    end
end