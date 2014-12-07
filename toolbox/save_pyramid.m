function [] = save_pyramid( pyr, cache_dir, idx )
%SAVE_PYRAMID Save pyramid data to disk with corresponding gist and blocks
    fname = [idx '.mat'];
    for i = 1:length(pyr)
        fprintf('- level %d of %d\n', i, length(pyr));
        % get cache directory for this level
        cache_level_dir = fullfile(cache_dir, num2str(i));
        % save pyramid level
        save_web_data(pyr{i}, cache_level_dir, fname);
        % save gist data
        g = imgist(pyr{i});
        save_web_data(g, cache_level_dir, 'gist', fname);
        % save blocks
        block_dir = fullfile(cache_level_dir, 'blocks', idx);
        save_image_blocks(pyr{i}, block_dir);
    end
end

