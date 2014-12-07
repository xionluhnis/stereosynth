% PYR_LEVELS - compute the number of pyramid levels
%
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function L = pyr_levels(im)
    % Figure out how many levels we need for the image pyramid
    % Ensures that smallest scale has resolution whose minimum dimension
    % (smallest row or column) is 16    
    min_size = min([size(im,1) size(im,2)]);
    L = floor(log(min_size) / log(2)) - 4;
end

%     [h, w, ~] = size(im);
%     side = min([h, w]);
%     L = 0;
%     while side > 35
%         L = L + 1;
%         side = ceil(L * 0.5);
%     end