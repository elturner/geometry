function ang = compute_compass_north(madfile, ic4file, timefile)
	% ang = compute_compass_north(madfile, ic4file, timefile)
	%
	%	This function will estimate the direction
	%	of magnetic north in the coordinate frame
	%	of the backpack localization represented
	%	in the given .mad file, by using the compass
	%	readings from the given ic4 .dat file.
	%
	% arguments:
	%
	%	madfile -	The location of the path to use
	%	ic4file -	The ic4 .dat file
	%	timefile -	The timestamp sync .xml file
	%
	% output:
	%
	%	ang -	Estimate of compass north (in radians counter-
	%		clockwise from +x axis)
	%

	% read files
	[poses,pose_times] = readMAD(madfile);
	[ic4dat,header]    = read_ic4_data(ic4file);
	timesync           = read_timesync_xml(timefile);

	% get synchronized timestamps for the ic4
	ic4_times = convert_to_system_time(timesync, 'IC4_1303271', ...
			ic4dat.timestamps);
	ic4_poses = get_nearest_poses(ic4_times, pose_times);
	T_comp = ic4dat.mag_body;	

	% we don't need the hardware config file, since we can assume
	% the IC4 is the same as system common coordinates (this is
	% not the best, but it's a proof-of-concept script)

	figure;
	render_path(poses);
	hold on;
	title('Each sample estimate');

	% iterate over readings
	T_north = [0;0;0];
	loc = zeros(2, length(ic4_poses));
	for i = 1:length(ic4_poses)

		% get best pose
		p = poses(4:6,ic4_poses(i));

		% get rotation matrix for this pose
		R = convert_NED_to_mat(p(1),p(2),p(3));

		% get world orientation for this pose's compass
		mag = norm(T_comp(:,i)); % we want to normalize the vector
		T_wld = R * T_comp(:,i) / mag;

		% this points in the positive magnetic field, which goes
		% from the north pole to the south pole.  By negating
		% this value, we can get an estimate of the direction to
		% north.
		T_north = T_north + -T_wld;
		loc(:,i) = poses(1:2, ic4_poses(i));
		plot(loc(1,i)+[0, -T_wld(1)], loc(2,i)+[0, -T_wld(2)],'r-');
	end
		
	plot(loc(1,:), loc(2,:), 'b-', 'LineWidth', 4);

	% get the average reading
	T_north = T_north / length(ic4_poses);
	ang = atan2(T_north(2), T_north(1));

	% plot value
	figure;
	render_path(poses);
	plot([0 cos(ang)], [0 sin(ang)], 'r-');
	plot(0,0,'bo');
	title('Final estimate of Magnetic North');
end
