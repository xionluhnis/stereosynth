function N = save_image_blocks( im, dir, blk_size )
%SAVE_IMAGE_BLOCKS Save image blocks to disk for caching
    if ~exist(dir, 'dir')
        mkdir(dir);
    end
    
    if nargin < 3
        blk_size = [16, 16];
    end
    
    % split into blocks
    [h, w, ~] = size(im);
    n_h = ceil(h / blk_size(1));
    n_w = ceil(w / blk_size(2));
    for r = 1:blk_size(1):h
        h_range = r:min([h, r+blk_size(1)-1]);
        for c = 1:blk_size(2):w
            w_range = c:min([w, c+blk_size(2)-1]);
            blk = im(h_range, w_range, :);
            save_web_data(blk, dir, sprintf('%d_%d.mat', r, c));
        end
    end
    N = n_h * n_w;
end

