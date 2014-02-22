function [] = render_box(bounds)
	% render_box(bounds)
	%
	%	Will render an axis-aligned box in 3D space from the
	%	specified bounds of the box.
	%
	% arguments:
	%
	%	bounds -	A 3x2 matrix, where:
	%
	%				[ xmin  xmax ;
	%				  ymin  ymax ;
	%				  zmin  zmax ]
	%

	% get input
	xmin = bounds(1,1);
	xmax = bounds(1,2);
	ymin = bounds(2,1);
	ymax = bounds(2,2);
	zmin = bounds(3,1);
	zmax = bounds(3,2);

	% get edges
	xs = [	xmin xmax;
		xmax xmax;
		xmax xmin;
		xmin xmin;
		xmin xmin;
		xmax xmax;
		xmax xmax;
		xmin xmin;
		xmin xmax;
		xmax xmax;
		xmax xmin;
		xmin xmin];
	ys = [	ymin ymin;
		ymin ymax;
		ymax ymax;
		ymax ymin;
		ymin ymin;
		ymin ymin;
		ymax ymax;
		ymax ymax;
		ymin ymin;
		ymin ymax;
		ymax ymax;
		ymax ymin];
	zs = [	zmin zmin;
		zmin zmin;
		zmin zmin;
		zmin zmin;
		zmin zmax;
		zmin zmax;
		zmin zmax;
		zmin zmax;
		zmax zmax;
		zmax zmax;
		zmax zmax;
		zmax zmax];

	% plot each edge
	hold all;
	axis equal;
	for i = 1:size(xs, 1)
		plot3(xs(i,:), ys(i,:), zs(i,:), '-k', 'LineWidth', 2);
	end
	xlabel('x');
	ylabel('y');
	zlabel('z');
end
