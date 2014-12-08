function [] = save_mat( data, varargin )
%SAVE_MAT Save data to disk
    fname = fullfile(varargin{:});
    % create parent directory if needed
    p = fileparts(fname);
    if ~exist(p, 'dir')
        mkdir(p);
    end
    % save mat data
    save(fname, 'data');
end
