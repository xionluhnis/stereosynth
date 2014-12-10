function img = load_img(file)
    if ends_with(file, '.mat')
        img = load_mat(file); 
    else
        img = im2double(imread(file));
    end
end

function res = ends_with(str, pat)
    S = length(str);
    P = length(pat);
    if S < P
        res = 0;
    else
        res = strcmp(str(end-P+1:end), pat);
    end
end