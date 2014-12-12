function [left, right] = get_frames(img, rescale)
    h0 = floor(size(img, 1) * 0.5);
    h1 = ceil(size(img, 1) * 0.5);
    left = img(1:h0, :, :);
    right = img(h1+1:end, :, :);
    
    if nargin >= 2
        left = imresize(left, rescale);
        right = imresize(right, rescale);
    end
end