function path = read_noisypath(filename)
	% path = read_noisypath(filename)
	%
	%	Parses the given .noisypath file and returns 
	%	a structure containing its info
	%
	% arguments:
	%
	%	filename -	The file to read and parse
	%
	% output:
	%
	%	The output structure that represents the data
	%	in this file.  It contains the following fields:
	%
	%		zupts -	List of zupts defined in file
	%		poses -	List of poses defined in file
	%
	%	Each zupt structure has the following fields:
	%
	%		start_time -	The start of the zupt interval (sec)
	%		end_time -	The end of the zupt interval (sec)
	%
	%	Each pose structure has the following fields:
	%
	%		timestamp -	The timestamp of this pose (sec)
	%		pos_mean -	The position mean (size: 3x1,
	%				units: meters)
	%		pos_cov -	The position covariance matrix
	%				(size: 3x3)
	%		rot_mean -	Rotational mean (roll, pitch, yaw),
	%				all angles are in radians
	%		rot_cov -	The rotational covariance matrix,
	%				(size: 3x3)
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	May 19th, 2014
	%

	% attempt to open binary file
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['Unable to open file: ', filename]);
	end

	% read magic number
	magic = fread(fid, 10, 'char')';
	desired_magic = ['noisypath', 0];
	if(~strcmp(char(magic), desired_magic))
		fclose(fid);
		error(['Invalid file format.  ', ...
			'Expected "', desired_magic, '"\t', ... 
			'Given file had magic number(', ...
			num2str(length(magic)), ...
			'): "', magic, '"']);
	end

	% read header information
	num_zupts = fread(fid, 1, 'uint32');
	num_poses = fread(fid, 1, 'uint32');

	% read zupt information
	if(num_zupts == 0)
		% put empty list
		path.zupts = [];
	else
		% iterate over zupts in file
		for i = 1:num_zupts
			path.zupts(i).start_time = fread(fid,1,'double');
			path.zupts(i).end_time   = fread(fid,1,'double');
		end
	end

	% read poses
	for i = 1:num_poses
		
		path.poses(i).timestamp    = fread(fid,1,'double');
		
		path.poses(i).pos_mean     = zeros(3,1);
		path.poses(i).pos_mean(1)  = fread(fid,1,'double');
		path.poses(i).pos_mean(2)  = fread(fid,1,'double');
		path.poses(i).pos_mean(3)  = fread(fid,1,'double');
		
		path.poses(i).pos_cov      = zeros(3,3);
		path.poses(i).pos_cov(1,1) = fread(fid,1,'double');
		path.poses(i).pos_cov(1,2) = fread(fid,1,'double');
		path.poses(i).pos_cov(1,3) = fread(fid,1,'double');
		path.poses(i).pos_cov(2,2) = fread(fid,1,'double');
		path.poses(i).pos_cov(2,3) = fread(fid,1,'double');
		path.poses(i).pos_cov(3,3) = fread(fid,1,'double');
		path.poses(i).pos_cov(2,1) = path.poses(i).pos_cov(1,2);
		path.poses(i).pos_cov(3,1) = path.poses(i).pos_cov(1,3);
		path.poses(i).pos_cov(3,2) = path.poses(i).pos_cov(2,3);

		path.poses(i).rot_mean     = zeros(3,1);
		path.poses(i).rot_mean(1)  = fread(fid,1,'double');
		path.poses(i).rot_mean(2)  = fread(fid,1,'double');
		path.poses(i).rot_mean(3)  = fread(fid,1,'double');
		
		path.poses(i).rot_cov      = zeros(3,3);
		path.poses(i).rot_cov(1,1) = fread(fid,1,'double');
		path.poses(i).rot_cov(1,2) = fread(fid,1,'double');
		path.poses(i).rot_cov(1,3) = fread(fid,1,'double');
		path.poses(i).rot_cov(2,2) = fread(fid,1,'double');
		path.poses(i).rot_cov(2,3) = fread(fid,1,'double');
		path.poses(i).rot_cov(3,3) = fread(fid,1,'double');
		path.poses(i).rot_cov(2,1) = path.poses(i).rot_cov(1,2);
		path.poses(i).rot_cov(3,1) = path.poses(i).rot_cov(1,3);
		path.poses(i).rot_cov(3,2) = path.poses(i).rot_cov(2,3);
	end

	% cleanup
	fclose(fid);
end
