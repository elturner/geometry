function data = read_color_image_metadata(filename)
	% data = read_color_image_metadata(filename)
	%
	%	Parses the specified metadata file
	%
	% arguments:
	%
	%	filename -	The metadata file for color images
	%
	% output:
	%
	%	data -	A struct that contains the following elements:
	%
	%		name         -  The name of the camera
	%		num_images   -  The number of images referenced
	%		jpeq_quality -  Quality of compression for these
	%		                images (range [1-100])
	%		output_dir   -  The directory to which images
	%		                were written
	%
	%		calibration  -	A struct that contains all
	%		                fisheye intrinsic calibration info
	%
	%		img_indices  - A Nx1 array of image indices
	%		img_files    - A Nx1 cell array of image filenames
	%		img_times    - A Nx1 array of timestamps
	%		               (units: seconds)
	%		img_exposure - A Nx1 array of exposure times (us)
	%		img_gain     - A Nx1 array of gains (range [1-4])
	%

	% open file for reading
	fid = fopen(filename);
	if(fid < 0)
		error(['could not open file: ', filename]);
	end

	% read header information
	data.name         = fgetl(fid);
	data.num_images   = str2num(fgetl(fid));
	data.jpeg_quality = str2num(fgetl(fid));
	data.output_dir   = fgetl(fid);

	% read calibration info
	pol = sscanf(fgetl(fid), '%f', Inf);
	data.calibration.poly = pol(2:end);
	if(length(pol) ~= 1 + pol(1))
		fclose(fid);
		error('polynomial coefficients for fisheye invalid');
	end

	invpol = sscanf(fgetl(fid), '%f', Inf);
	data.calibration.invpoly = invpol(2:end);
	if(length(invpol) ~= 1 + invpol(1))
		fclose(fid);
		error('inv polynomial coefficients for fisheye invalid');
	end
	
	data.calibration.center = sscanf(fgetl(fid), '%f %f');
	data.calibration.skew = sscanf(fgetl(fid), '%f %f %f');
	data.calibration.size = sscanf(fgetl(fid), '%d %d');

	% initialize lists
	data.img_indices  = [];
	data.img_files    = [];
	data.img_times    = [];
	data.img_exposure = [];
	data.img_gain     = [];

	% header should end with empty line
	e = fgetl(fid);
	if(~isempty(e))
		fclose(fid);
		error(['The header for given file is longer ', ...
		       'than expected: ', filename]);
	end

	% read the image properties
	while(~feof(fid))

		% read next line
		line = fgetl(fid);
		if(isempty(line))
			continue;
		end

		% split the line based on spaces
		spaces = find(line == ' ');
		if(length(spaces) < 4)
			fclose(fid);
			fprintf('num spaces = %d\n', length(spaces));
			error(['File formatted incorrectly: ', filename]);
		end

		% parse the line
		data.img_indices(end+1)    = str2num(line(1:spaces(1)-1));
		data.img_files{end+1}       = line(...
		                              spaces(1)+1:spaces(2)-1);
		data.img_times(end+1)        = str2num(line(...
		                              spaces(2)+1:spaces(3)-1));
		data.img_exposure(end+1)    = str2num(line(...
		                              spaces(3)+1:spaces(4)-1));
		data.img_gain(end+1)        = str2num(line(...
		                              spaces(4)+1:end));
	end

	% clean up
	fclose(fid);
end
