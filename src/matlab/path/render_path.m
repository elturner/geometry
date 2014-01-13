function [] = render_path(path, color)
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

	hold all;
	axis equal;

	plot(path(1,:), path(2,:), '-xm');

	if(exist('color', 'var'))
		
		% plot 'color' for each pose
		stem3(path(1,:), path(2,:), color);

	end
end
