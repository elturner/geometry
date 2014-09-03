function [] = render_edge(edgefile)
	% render_edge(edgefile)
	%
	%	Will render the edges provided in the given .edge
	%	formatted file.
	%
	%	Each edge file should have a list of 2D line segments
	%	where each line in the file represents a single segment
	%	as follows:
	%
	%		<x_start> <y_start> <x_end> <y_end>
	%
	%	This positions are assumed to be in units of meters.
	%
	% arguments:
	%
	%	edgefile -	The file to read
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Sept. 3, 2014
	%

	% read in the file
	E = load(edgefile);

	% render it
	hold on;
	axis equal;
	line(E(:,[1 3])', E(:,[2 4])');
end
