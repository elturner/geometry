function [] = render_carve_map(scan)
	% render_carve_map(scan)
	%
	%	Renders the carve map represented by
	%	a laser scan, given the distributions of the saensor and
	%	scan point positions.
	%
	%	At a particular position in this output map, the value
	%	is the probability that position is intersected by
	%	the scan.
	%
	% arguments:
	%
	%	scan - The scan statistics
	%

	% prepare the figure
	figure;
	hold all;
	set(gcf, 'renderer', 'opengl');

	% prepare sampling map
	x = linspace(scan.xmin,scan.xmax,60);
	y = linspace(scan.ymin,scan.ymax,60);
	[X,Y] = meshgrid(x,y);
	area = (x(2) - x(1));

	% sample
	Z = zeros(size(X));
	for i = 1:numel(Z)
		Z(i) = compute_carve_map(scan, [X(i);Y(i)], area);
	end

	% plot it
	surf(X,Y,Z);
end
