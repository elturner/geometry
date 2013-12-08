function data = read_dalsa_metadata(filename)
	% data = read_dalsa_metadata(filename)
	%
	%	Parses the specified metadata file
	%
	% arguments:
	%
	%	filename -	The metadata file for a dalsa camera
	%
	% output:
	%
	%	data -	A struct that contains the following elements:
	%
	%		name      -  The name of the camera
	%		serial    -  The serial number of the camera
	%		model     -  The model number of the camera
	%		img_dir   -  The image directory
	%
	%		img_files   - A Nx1 cell array of image filenames
	%		img_indices - A Nx1 array of image indices
	%		img_cam_time - A Nx1 array of camera timestamps (us)
	%		img_exposure - A Nx1 array of exposure times (us)
	%		img_gain     - A Nx1 array of gains (range [1-4])
	%

	% open file for reading
	fid = fopen(filename);
	if(fid < 0)
		error(['could not open file: ', filename]);
	end

	% check magic number to verify that this is correct filetype
	magic = fgetl(fid);
	if(~strcmp(magic, 'dalsa'))
		fclose(fid);
		error(['The specified file is not a dalsa ', ...
		       'metadata file: ', filename]);
	end
	
	% read header information
	data.name         = fgetl(fid);
	data.serial       = fgetl(fid);
	data.model        = fgetl(fid);
	data.img_dir      = fgetl(fid);
	data.img_files    = [];
	data.img_indices  = [];
	data.img_cam_time = [];
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
		data.img_files{end+1}       = line(1:spaces(1)-1);
		data.img_indices(end+1)     = str2num(line(...
		                              spaces(1)+1:spaces(2)-1));
		data.img_cam_time(end+1)    = str2num(line(...
		                              spaces(2)+1:spaces(3)-1));
		data.img_exposure(end+1)    = str2num(line(...
		                              spaces(3)+1:spaces(4)-1));
		data.img_gain(end+1)        = str2num(line(...
		                              spaces(4)+1:end));
	end

	% clean up
	fclose(fid);
end
