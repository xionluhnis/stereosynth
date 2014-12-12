function [ I ] = copy_img( input_file, output_file )
    I = imread(input_file);
    imwrite(I, output_file);
end

