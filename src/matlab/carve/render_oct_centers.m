function [] = render_oct_centers(oct)
	% render_oct_centers(oct)
	%
	%	Will render the center points for each
	%	octnode given.
	%

	% prepare figure
	figure;
	hold all;
	axis equal;
	set(gcf, 'renderer', 'opengl');

	% get centers
	c = [oct.center];
	h = [oct.halfwidth];
	d = [oct.data.count];
	leafs = [oct.isleaf];
	scatter3(c(1,leafs), c(2,leafs), c(3,leafs), 10, d(leafs));
end
