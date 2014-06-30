function [] = render_noisypath(path)
	% render_noisypath(path)
	%
	%	Renders the path that has been imported from a call
	%	to read_noisypath(...)
	%
	% arguments:
	%
	%	path - 	The path structure to render
	%
    
	% prepare figure
	hold all;
	axis equal;
    set(gcf, 'renderer', 'opengl');

	% iterate over path
    p = [path.poses.pos_mean];
    plot3(p(1,:), p(2,:), p(3,:), 'b-o');
end
