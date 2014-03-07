function [] = render_line_pdf(scan)
	% render_line_pdf(scan)
	%
	%	Renders the pdf of the line segment representing
	%	a laser scan, given the distributions of the saensor and
	%	scan point positions.
	%
	% arguments:
	%
	%	scan - The scan statistics

	% prepare the figure
	figure;
	hold all;
	set(gcf, 'renderer', 'opengl');

	% prepare sampling map
	[X,Y] = meshgrid(linspace(scan.xmin,scan.xmax,60), ...
	                 linspace(scan.ymin,scan.ymax,60));

	% sample
	Z = zeros(size(X));
	for i = 1:numel(Z)
		Z(i) = compute_line_pdf(scan, [X(i);Y(i)]);
	end

	% plot it
	surf(X,Y,Z);

	% check normalization
	x = unique(X);
	y = unique(Y);
	a = (x(2)-x(1))*(y(2)-y(1));
	fprintf('line dist normalized? %f\n', a*sum(Z(:)));
end
