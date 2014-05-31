function [] = convert_mad_to_noisypath(madfile, noisypathfile, err_T, err_R)
	% convert_mad_to_noisypath(madfile, noisypathfile, err_T, err_R)
	%
	%	Will generate a .noisypath file from a given .mad file
	%	by assuming every pose has uniform translation error err_T
	%	and uniform rotational error err_R.
	%
	% arguments:
	%
	%	madfile -	The path to the .mad file to use
	%	noisypathfile -	The output .noisypath file
	%	err_T -		The standard-deviation to use for position
	%	err_R -		The standard-deviation to use for rotation
	%

	% read the mad file
	[poses, timestamps, zupts] = readMAD(madfile);

	% prepare output file
	fid = fopen(noisypathfile, 'wb');
	if(fid < 0)
		error(['Unable to open file for writing: ', noisypathfile]);
	end

	% get sizes of lists
	num_poses = size(poses, 2);
	num_zupts = size(zupts, 1);

	% write header information to file
	fwrite(fid, ['noisypath',0], 'char');
	fwrite(fid, num_zupts, 'uint32');
	fwrite(fid, num_poses, 'uint32');

	% write zupt information to file
	for i = 1:num_zupts
		fwrite(fid, zupts(i,1), 'double');
		fwrite(fid, zupts(i,2), 'double');
	end

	% write pose information to file
	var_T = err_T * err_T;
	var_R = err_R * err_R;
	for i = 1:num_poses
		
		% write timestamp
		fwrite(fid, timestamps(i), 'double');

		% write position information, units of meters
		fwrite(fid, poses(1,i), 'double'); % x
		fwrite(fid, poses(2,i), 'double'); % y
		fwrite(fid, poses(3,i), 'double'); % z

		% write the covariance matrix.  Note that we're making
		% up this data given the input.
		fwrite(fid, var_T, 'double'); % Cov(X,X)
		fwrite(fid, 0,     'double'); % Cov(X,Y)
		fwrite(fid, 0,     'double'); % Cov(X,Z)
		fwrite(fid, var_T, 'double'); % Cov(Y,Y)
		fwrite(fid, 0,     'double'); % Cov(Y,Z)
		fwrite(fid, var_T, 'double'); % Cov(Z,Z)
		
		% write rotation angles (still in NED coordinates)
		fwrite(fid, poses(4,i), 'double'); % roll
		fwrite(fid, poses(5,i), 'double'); % pitch
		fwrite(fid, poses(6,i), 'double'); % yaw

		% the covariance matrices for the rotations are also
		% 'made up' in this context.
		fwrite(fid, var_R, 'double'); % Cov(roll,  roll)
		fwrite(fid, 0,     'double'); % Cov(roll,  pitch)
		fwrite(fid, 0,     'double'); % Cov(roll,  yaw)
		fwrite(fid, var_R, 'double'); % Cov(pitch, pitch)
		fwrite(fid, 0,     'double'); % Cov(pitch, yaw)
		fwrite(fid, var_R, 'double'); % Cov(yaw,   yaw)
	end

	% clean up
	fclose(fid);
end
