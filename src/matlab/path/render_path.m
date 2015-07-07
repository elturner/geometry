function [] = render_path(path, color, labels, axes_sep)
	% render_path(path)
	%
	%
	%	Will draw path on current figure, where
	%	path was generated using:
	%
	%		path = readMAD(filename);
	%
	% arguments:
	%
	%	path -	The list of poses to draw
	%
	%	color -	Optional.  A 1xN array, with scalar values
	%		associated with each pose.
	%
	%	labels - Optional.  A 1xN array of numbers to write
	%		 on the path.
	%
	%	axes_sep - Optional.
	%                  Shows the orientation of the system every
	%		   this-many poses.  If negative, no axes
	%		   drawn.  Negative -> do not draw.
	%

	hold all;
	axis equal;

	% plot the path
	plot3(path(1,:), path(2,:), path(3,:), '-xm');

	% check if we should plot labels
	if(exist('labels', 'var') && length(labels) == length(path))
		for i = 1:size(path, 2)
			text(path(1,i), path(2,i), num2str(labels(i)));
		end
	end

	% check for stemplot
	if(exist('color', 'var') && ~isempty(color))
		
		% plot 'color' for each pose
		stem3(path(1,:), path(2,:), color);

	end

	% plot orientation along path
	if(exist('axes_sep', 'var') && axes_sep > 0)
		
		% draw axes
		for i = 1:axes_sep:length(path)
			
			% get the translation
			T = path(1:3,i);

			% get the rotation
			R = convert_NED_to_mat(path(4:6,i));

			% plot it
			render_axis(0.5, T, R);
		end
	end
end

% helper function: render a single axis given 6 degrees of freedom
% 	s is the size of the axis to render,
% 	T is the translation vector, 
% 	R is the rotation matrix
function [] = render_axis(s, T, R)

	hold all;

	% apply transformation to axes
	x_axis = R*[s;0;0] + T;
	y_axis = R*[0;s;0] + T;
	z_axis = R*[0;0;s] + T;

	% render the new axes
	plot3([T(1), x_axis(1)], ...
	      [T(2), x_axis(2)], ...
	      [T(3), x_axis(3)], 'r-', 'LineWidth', 2);
	plot3([T(1), y_axis(1)], ...
	      [T(2), y_axis(2)], ...
	      [T(3), y_axis(3)], 'g-', 'LineWidth', 2);
	plot3([T(1), z_axis(1)], ...
	      [T(2), z_axis(2)], ...
	      [T(3), z_axis(3)], 'b-', 'LineWidth', 2);
end
