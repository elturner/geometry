function [] = render_hia(hia)
	% RENDER_HIA(hia)
	%
	%	Renders the contents of the hia structure
	%	to a figure.
	%
	%	This structure should be populated by a call
	%	to read_hia(filename).
	%
	% arguments:
	%
	%	hia -	The Histogrammed Interior Area (HIA) structure
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on February 28th, 2015
	%

	% prepare the figure
	hold on;
	axis equal;

	% prepare the pixels
	pixels_x = ([-1;-1;1;1] * 0.5 * hia.resolution) ...
				* ones(1,hia.num_cells) ...
				+ [1;1;1;1] * hia.centers(:,1)';
	pixels_y = ([1;-1;-1;1] * 0.5 * hia.resolution) ...
				* ones(1,hia.num_cells) ...
				+ [1;1;1;1] * hia.centers(:,2)';

	% render the patches for each pixel
	values = hia.open_heights';
	patch(pixels_x, pixels_y, values, 'LineStyle', 'None');

end
