function im_rgb = laby2rgb( im, lambda )
%LABY2RGB Transform from CIE La*b*+y back to RGB
    assert(isfloat(im), 'Only floating point images supported');
    if nargin < 2
        lambda = 1;
    else
        assert(lambda > 0, 'Invalid lambda, should be positive!');
    end
    cform = makecform('lab2srgb');
    if isa(im, 'single')
        im = double(im);
    end
    im_lab = cat(3, ...
        100 * im(:, :, 1), ...
        160 * im(:, :, 2) - 80, ...
        160 * im(:, :, 3) - 80 ...
    );
    im_rgb = applycform(im_lab, cform);
end

