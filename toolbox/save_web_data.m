function [] = save_web_data( D, varargin )
%SAVE_WEB_DATA Save data to disk
    fname = fullfile(varargin{:});
    % create parent directory if needed
    p = fileparts(fname);
    if ~exist(p, 'dir')
        mkdir(p);
    end
    % save mat data
    save(fname, 'D');
end

