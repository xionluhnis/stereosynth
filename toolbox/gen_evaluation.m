
addpath toolbox

vector=@(v)v(:);
errorEval = @(synth, truth)mean(vector(synth-truth).^2);

% load image
src_dir = 'results/1205/web/test';
images = find_images(src_dir);
N = length(images);
out_dir = 'results/1210/all';
params = 1:12;
if exist('results.mat', 'file')
    res = load_mat('results.mat');
else
    res = cell(N, 14);
    for i = 1:N
        file = images{i};
        [~, name, ~] = fileparts(file);
        % fprintf('Processing %s (%d of %d)\n', name, i, length(images));
        frames = single(im2double(imread(file)));
        [left, right] = get_frames(frames, 0.25);

        % frame name
        res{i, 1} = name;

        % left result
        res{i, 2} = errorEval(left * 255, right * 255);
        fprintf('%s & %.2f ', name, res{i, 2});

        for p = params
            result_dir = fullfile(out_dir, name);
            result_file = fullfile(result_dir, ['res' num2str(p) '.mat']);
            synth = load_mat(result_file, 'synth');

            res{i, 2+p} = errorEval(right * 255, synth  * 255);
            fprintf('& %.2f ', res{i, 2+p});
        end
        fprintf('\\\\\n');
    end

    save_mat(res, 'results.mat');
end



% generate result table and data
fprintf('Frame & Incremental & Left & PG & PL & DG & DL & FG & FL\\\\\\hline\n');
report_dir = 'report/figures/stereo';
for i = 1:N
    data = res(i, :);
    name = strrep(data{1}, '_frame-', '');
    fname = fullfile(src_dir, [data{1} '.jpg']);
    eol = '';
    for incr = [1, 0]
        incr_str = {'no', 'yes'};
        fprintf('%s & %s & %.2f ', name, incr_str{incr+1}, data{2}); name = '';
        
        subp = [3, 4, 7, 8, 11, 12];
        if ~incr
            subp = subp + 2;
        end
        for p = subp
            fprintf('& %.2f ', data{p});
        end
        fprintf('\\\\%s\n', eol); eol = '\hline';
    end
    
    file = images{i};
    [~, name, ~] = fileparts(file);
    result_dir = fullfile(out_dir, name);
    % fprintf('Processing %s (%d of %d)\n', name, i, length(images));
    frames = single(im2double(imread(file)));
    [left, right] = get_frames(frames, 0.25);
    imwrite(right, fullfile(report_dir, [name '-0.jpg']));
    for p = params
        copy_img(fullfile(result_dir, ['right-' num2str(p) '.png']), ...
                 fullfile(report_dir, [name '-' num2str(p) '.jpg']));
    end
end