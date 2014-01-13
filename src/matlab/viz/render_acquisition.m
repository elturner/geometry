function [] = render_acquisition(configfile, pathfile, timefile, toffile)
	% render_acquisition(configfile, pathfile, timefile, toffile)
	%
	%	Will render the positions of scanners along the
	%	recovered path of the data acquisition
	%
	% arguments:
	%
	%	configfile -	Hardware config xml file for backpack
	%	pathfile -	The .mad file for the 3D path
	%
	% optional arguments:
	%
	%	timefile -	The time sync xml file
	%	toffile -	The d-imager .dat file
	%

	% need additional parser functions
	addpath('../config/');
	addpath('../path/');
	addpath('../timesync/');
	addpath('../sensors/d_imager/');
	addpath('../sensors/urg/');

	% read in files
	conf = read_hardware_config(configfile);
	[poses, posetimes] = readMAD(pathfile);
	num_poses = size(poses, 2);

	% get same scans
	if(exist('timefile', 'var') && exist('toffile', 'var'))
	
		% read time sync
		timesync = read_timesync_xml(timefile);
		
		% read tof scanner
		tof_scans = read_d_imager_scandata(toffile);
		tof_pose_inds = nearest_poses_for_TOF(...
		                tof_scans,posetimes,timesync);
	end
	
	% test data for urg
	theta = linspace(-135,135,1000) * pi / 180;
	URG = [cos(theta);sin(theta);zeros(size(theta))];
	clear theta;

	% prepare figure
	figure(1);
	set(gcf, 'renderer', 'opengl');

	% iterate over poses
	for i = [1:num_poses]
		
		% get transform of system -> world for this pose
		T = poses(1:3,i); % translation, units: meters
		R = get_rotation_matrix(poses(4:6,i)); % rotation matrix

		% get list of nearby poses
		near_poses = [max(1,i-20):1:min(num_poses,i+20)];

		% display system position
		figure(1);
		[az,el] = view();
		hold off;
		plot3(T(1), T(2), T(3), 'xk', 'LineWidth', 3);
		hold on;
		plot3(poses(1,near_poses), ...
		      poses(2,near_poses), ...
		      poses(3,near_poses), 'k-');

		% display sensor positions

		% cameras
		for j = [1:length(conf.cameras)]
			
			% camera -> system translation, units: meters
			tc = conf.cameras(j).tToCommon * 0.001; 
		
			% get camera position in world coordinates
			tw = R*tc + T;
			rw = R*conf.cameras(j).rToCommon;

			% display this
			plot3(tw(1), tw(2), tw(3), 'bo', 'LineWidth', 2);
		end

		% lasers
		for j = [1:length(conf.lasers)]
			
			% laser -> system translation, units: meters
			tc = conf.lasers(j).tToCommon * 0.001; 
		
			% get laser position in world coordinates
			tw = R*tc + T;
			rw = R*conf.lasers(j).rToCommon;

			% display this
			plot3(tw(1), tw(2), tw(3), 'ro', 'LineWidth', 2);
			
			% display sample scan
			URG_world = rw*URG + tw*ones(1,size(URG,2));
			plot3(URG_world(1,:), ...
				URG_world(2,:), ...
				URG_world(3,:), 'r.');
		end
	
		% tof cameras
		for j = [1:length(conf.tof_cameras)]
			
			% tof -> system translation, units: meters
			tc = conf.tof_cameras(j).tToCommon * 0.001; 
		
			% get tof position in world coordinates
			tw = R*tc + T;
			rw = R*conf.tof_cameras(j).rToCommon;

			% display this
			plot3(tw(1), tw(2), tw(3), 'mo', 'LineWidth', 2);
			
			% display scans if available
			if(exist('tof_pose_inds', 'var'))

				% retrieve closest scan
				tpi = tof_pose_inds(i);
				TOF = [ tof_scans.scans(tpi).xdat' ;
				       	tof_scans.scans(tpi).ydat' ;
				        tof_scans.scans(tpi).zdat' ]*0.001; 
					% units: meters
	
				% transform to world coordinates
				TOF_world = rw*TOF + tw*ones(1,size(TOF,2));
				plot3(TOF_world(1,:), ...
					TOF_world(2,:), ...
					TOF_world(3,:), 'm.');
			end
		end

		% show user everything for this pose
		axis equal;
		view(az, el);
		drawnow;
	end
end

% given euler angles in degrees and NED coordinates, 
% computes rotation matrix
function R = get_rotation_matrix(r)

	roll = r(1);
	pitch = r(2);
	yaw = r(3);

	% construct a rotation matrix
	Rx = [1, 0,          0;
	      0, cos(roll), -sin(roll);
	      0, sin(roll),  cos(roll)];

	Ry = [ cos(pitch), 0, sin(pitch);
	       0,          1,          0;
	      -sin(pitch), 0, cos(pitch)];

	Rz = [cos(yaw), -sin(yaw), 0;
	      sin(yaw),  cos(yaw), 0;
	      0       ,  0       , 1];

	NED2ENU = [	0  1  0 ;
			1  0  0 ;
			0  0 -1 ];

	% save rotation matrix
	R = (NED2ENU * Rz * Ry * Rx);
end

% get nearest pose indices for each scan in the tof scan structure
function pose_inds = nearest_poses_for_TOF(tof_scans,posetimes,timesync)

	% prepare output array
	pose_inds = zeros(length(tof_scans), 1);

	% get the index in timesync that applies to the TOF sensor
	ts_ind = 0;
	for i = 1:length(timesync)
		if(strcmp(timesync(i).name, 'D-Imager'))
			ts_ind = i;
			break;
		end
	end

	% iterate over scans
	for i = [1:length(tof_scans.scans)]
		
		% get synced timestamp for this scan frame
		t = timesync(ts_ind).slope ...
			* tof_scans.scans(i).timestamp...
			+ timesync(ts_ind).offset;

		% get the pose index that is closest to this time
		[y,j] = min(abs(t - posetimes));
		pose_inds(i) = j;
	end
end
