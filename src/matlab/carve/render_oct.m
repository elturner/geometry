function [] = render_oct(oct)
	% render_oct(oct)
	%
	%	Will render the specified octree structure
	%

	% start at the root node, which is the first element
	% in the list, and recursively find all the leaves and
	% render them
	hold all;
	axis equal;
	set(gcf, 'renderer', 'opengl');
	render_node(oct, 1);

end

function [] = render_node(oct, i)
	% render_node(oct, i)
	%
	%	Renders the node and subnodes of #i
	%

	% iterate through children, render if present
	leaf = true;
	for j = 1:8
		if(oct(i).children(j) > 0)
			% i'th node not a leaf, render children
			leaf = false;
			render_node(oct, oct(i).children(j));
		end
	end

	% render this node if it is a leaf
	if(leaf)
		render_cube(oct(i).center, oct(i).halfwidth);
	end
end

function [] = render_cube(center, halfwidth)
	% render_cube(center, halfwidth)
	%
	%	Will render a cube given the center and halfwidth
	%

	% get input
	xmin = center(1) - halfwidth;
	xmax = center(1) + halfwidth;
	ymin = center(2) - halfwidth;
	ymax = center(2) + halfwidth;
	zmin = center(3) - halfwidth;
	zmax = center(3) + halfwidth;

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
	for i = 1:size(xs, 1)
		plot3(xs(i,:), ys(i,:), zs(i,:), '-k', 'LineWidth', 2);
	end
	xlabel('x');
	ylabel('y');
	zlabel('z');
end
