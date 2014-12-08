function [left, right] = get_frames(img)
    h0 = floor(size(img, 1) * 0.5);
    h1 = ceil(size(img, 1) * 0.5);
    left = img(1:h0, :, :);
    right = img(h1+1:end, :, :);
end