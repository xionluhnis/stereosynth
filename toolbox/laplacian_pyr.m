%LAPLACIAN_PYR Compute a laplacian pyramid from a gaussian pyramid
%               from 1=coarsest to N=finest level
%
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function pyr = laplacian_pyr( gauss_pyr )
    levels = length(gauss_pyr);
    pyr = cell(1, levels);
    % first scale is the same as gaussian
    pyr{1} = gauss_pyr{1};
    % other scales are frequency bands
    for i = 2:levels
        low_freq = pyr_expand(gauss_pyr{i-1}, size(gauss_pyr{i}));
        pyr{i} = gauss_pyr{i} - low_freq;
    end
end