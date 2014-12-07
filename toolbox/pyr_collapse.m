% Given a Laplacian pyramid, starting from the smallest scale,
% expand it, then add to upper scale.  Repeat this until we get to the
% original scale and this is the output
%
% BASED ON
%   https://github.com/rayryeng/laplacianBlend
%
function out = pyr_collapse(Lpyr)
    stack = Lpyr;
    levels = length(Lpyr);
    for i = 1:levels-1
        stack{i+1} = stack{i+1} + pyr_expand(stack{i}, size(stack{i+1}));
    end

    out = stack{levels};
end
