function conf = parse_transforms(conf)
	% conf = parse_transforms(conf)
	%
	%	Replaces all sensors' rotation and translation
	%	fields with matlab matrices and vectors instead
	%	of comma-separated strings.
	%
	%	NOTE: this function is automatically called within
	%	      read_hardware_config(), so no other program
	%	      needs to explicitly call it.
	%
	% arguments:
	%
	%	conf -	Configuration struct from read_hardware_config()
	%
	% output:
	%
	%	conf -	The same structure, with rToCommon and rToCommon
	%		fields changed
	%

	if(isfield(conf, 'lasers'))
		for i = 1:length(conf.lasers)
			conf.lasers(i) = replace_sensor_transform(...
			                 conf.lasers(i));
		end
	end
	if(isfield(conf, 'tof_cameras'))
		for i = 1:length(conf.tof_cameras)
			conf.tof_cameras(i) = replace_sensor_transform(...
			                 conf.tof_cameras(i));
		end
	end
	if(isfield(conf, 'cameras'))
		for i = 1:length(conf.cameras)
			conf.cameras(i) = replace_sensor_transform(...
			                 conf.cameras(i));
		end
	end
	if(isfield(conf, 'imus'))
		for i = 1:length(conf.imus)
			conf.imus(i) = replace_sensor_transform(...
			                 conf.imus(i));
		end
	end
	if(isfield(conf, 'magnetometers'))
		for i = 1:length(conf.magnetometers)
			conf.magnetometers(i) = ...
				replace_magnetometer_transforms(...
				conf.magnetometers(i));
		end
	end
end

function sensor = replace_sensor_transform(sensor)
	
	% get the rotation
	r = sscanf(sensor.rToCommon, '%f, %f, %f');
	sensor.rToCommon = get_rotation_matrix(r);

	% get translation
	T = sscanf(sensor.tToCommon, '%f, %f, %f');
	sensor.tToCommon = T;
end

function mag = replace_magnetometer_transforms(mag)

	R0 = sscanf(mag.rToCommon_0, '%f, %f, %f');
	mag.rToCommon_0 = get_rotation_matrix(R0);
	T0 = sscanf(mag.tToCommon_0, '%f, %f, %f');
	mag.tToCommon_0 = T0;

	R1 = sscanf(mag.rToCommon_1, '%f, %f, %f');
	mag.rToCommon_1 = get_rotation_matrix(R1);
	T1 = sscanf(mag.tToCommon_1, '%f, %f, %f');
	mag.tToCommon_1 = T1;

	R2 = sscanf(mag.rToCommon_2, '%f, %f, %f');
	mag.rToCommon_2 = get_rotation_matrix(R2);
	T2 = sscanf(mag.tToCommon_2, '%f, %f, %f');
	mag.tToCommon_2 = T2;
	
	R3 = sscanf(mag.rToCommon_3, '%f, %f, %f');
	mag.rToCommon_3 = get_rotation_matrix(R3);
	T3 = sscanf(mag.tToCommon_3, '%f, %f, %f');
	mag.tToCommon_3 = T3;
end

% given euler angles in degrees, computes rotation matrix
function R = get_rotation_matrix(r)

	roll = r(1);
	pitch = r(2);
	yaw = r(3);

	% construct a rotation matrix
	roll = roll * pi / 180;
	pitch = pitch * pi / 180;
	yaw = yaw * pi / 180;
	
	Rx = [1, 0,          0;
	      0, cos(roll), -sin(roll);
	      0, sin(roll),  cos(roll)];

	Ry = [ cos(pitch), 0, sin(pitch);
	       0,          1,          0;
	      -sin(pitch), 0, cos(pitch)];

	Rz = [cos(yaw), -sin(yaw), 0;
	      sin(yaw),  cos(yaw), 0;
	      0       ,  0       , 1];
	
	% save rotation matrix
	R = (Rz * Ry * Rx);
end
