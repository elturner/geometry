function [] = render_windows(windows)
	% RENDER_WINDOWS(windows)
	%
	%	Given a set of windows, will
	%	generate a 2D plot of the window locations
	%	in the floorplan coordinate system.
	%
	% arguments:
	%
	%	windows -	A windows structure parsed using
	%			read_windows(windowsfile, fp).
	%
	%			An array of window structures can
	%			also be provided
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on January 13, 2015
	%

	% prepare figure
	hold on;
	axis equal;
	set(gcf, 'renderer', 'opengl');

	% iterate over structures
	for wi = 1:length(windows)

		% draw the windows
		for i = 1:windows(wi).num_windows

			% draw this window
			plot(windows(wi).verts2d(i,[1 3]), ...
				windows(wi).verts2d(i,[2 4]), ...
				'r-', 'LineWidth', 3);
		end
	end
end
