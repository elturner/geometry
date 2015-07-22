function M = read_color_image(dataset_dir, colormeta, img_index)
	%M = READ_COLOR_IMAGE(dataset_dir, colormeta, img_index)
	%
	% Will read the specified image into matlab as a RGB matrix
	%
	% Determines the appropriate image filepath from the 
	% specified information and will return the image data.
	%
	% arguments:
	%
	%	dataset_dir:	location of dataset on disk
	%
	%	colormeta:	Metadata structure from 
	%			read_color_image_metadata()
	%
	%	img_index:	The index of the image to read,
	%			should be in range [1, colormeta.num_images]
	%
	% output:
	%
	%	M:		3-channel RGB image from disk
	%
	% author:
	%
	%	Created by Eric Turner <elturner@indoorreality.com>
	%	June 25, 2015
	%

	% get full file path for specified image
	path = fullfile(dataset_dir, ...
			colormeta.output_dir, ...
			colormeta.img_files{img_index} );

	% read in the image
	M = imread(path);
end
