function [] = render_path(path, color, labels)
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

	% plot the path
	plot(path(1,:), path(2,:), '-xm');

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
end
