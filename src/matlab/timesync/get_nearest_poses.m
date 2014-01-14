function pose_inds = get_nearest_poses(sensor_times, path_times)

	% pose_inds = get_nearest_poses(sensor_times, path_times)
	%
	%	Will find the pose indices that are closest to when each
	%	sensor frame took place.
	%
	% arguments:
	%
	%	sensor_times -	The times of each sensor frame, in system
	%			common clock
	%
	%	path_times -	The times of each pose, in system common
	%			clock
	%
	% output:
	%
	%	pose_inds -	A mapping between indices of sensor_times
	%			to indices of path_times
	%

	% initialize pose lsit
	pose_inds = zeros(size(sensor_times));

	% iterate over input times
	for i = 1:length(sensor_times)
		
		% get nearest pose time
		[y,j] = min(abs(path_times - sensor_times(i)));
		pose_inds(i) = j;
	end
end
