function img_files = find_images( path )
    dir_files = dir(path);
    img_files = cell(0, 0);
    for i = 1:length(dir_files)
        if dir_files(i).isdir
            continue;
        end
        fname = fullfile(path, dir_files(i).name);
        if is_image(fname)
            img_files{end+1} = fname;
        end
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

function b = is_image(fname)
    f = imformats;
    for i = 1:length(f)
        if f(i).isa(fname)
            b = 1;
            return
        end
    end
    b = 0;
end