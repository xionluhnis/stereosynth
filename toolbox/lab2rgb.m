function im_rgb = lab2rgb( im )
%LAB2RGB Transform from CIE La*b* back to RGB
    assert(isfloat(im), 'Only floating point images supported');
    cform = makecform('lab2srgb');
    if isa(im, 'single')
        im = double(im);
    end
    im_lab = cat(3, ...
        100 * im(:, :, 1), ...
        200 * im(:, :, 2) - 100, ...
        200 * im(:, :, 3) - 100 ...
    );
    im_rgb = applycform(im_lab, cform);
end

