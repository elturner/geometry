function R = convert_NED_to_mat(roll, pitch, yaw)
	% R = convert_NED_to_mat(roll, pitch, yaw)
	% R = convert_NED_to_mat([roll, pitch, yaw])
	%
	% 	Given euler angles in radians and NED coordinates, 
	% 	computes rotation matrix in ENU coordinates.

	% check input format
	if(~exist('pitch', 'var') && ~exist('yaw', 'var'))
		pitch = roll(2);
		yaw = roll(3);
		roll = roll(1);
	end

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
