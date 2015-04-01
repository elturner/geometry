function [r,window_area,wall_area] ...
		= compute_window_to_wall_ratio(fp, windows)
	% r = COMPUTE_WINDOW_TO_WALL_RATIO(fp, windows)
	%
	%	Computes the window-to-wall ratio of the given floorplan
	%	with the specified windows.
	%
	%	NOTE: assumes all floor plan walls are exterior.
	%
	% arguments:
	%
	%	fp -	The floor plan structure
	%
	%	windows -	The windows structure
	%
	% output:
	%
	%	r -	The ratio of area
	%
	%	window_area -	Optional second argument:  total window
	%			area (units: meters squared)
	%
	%	wall_area -	Optional third argument: total wall area
	%			(units: meters squared)		
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	created:  March 31, 2015
	%

	% first compute total wall area
	wall_area = 0;
	for i = 1:size(fp.edges,1)

		% get the height of this wall
		wh = fp.edge_ceilings(i) - fp.edge_floors(i);

		% get width of wall
		v1 = fp.verts(fp.edges(i,1),:);
		v2 = fp.verts(fp.edges(i,2),:);
		ww = norm(v1 - v2);

		% add to total area
		wall_area = wall_area + (ww*wh);
	end

	% next compute window area
	window_area = 0;
	for i = 1:windows.num_windows
	
		% get wall index
		wi = find(windows.wall_inds(i,1) == fp.edges(:,1) ...
				& windows.wall_inds(i,2) == fp.edges(:,2));
		if(numel(wi) ~= 1)
			error('unable to uniquely determine wall');
		end

		% get wall horizontal positions
		v1 = fp.verts(windows.wall_inds(i,1),:);
		v2 = fp.verts(windows.wall_inds(i,2),:);

		% get width of window
		ww = norm(v1 - v2) * (windows.max_horizontal(i) ...
					- windows.min_horizontal(i));

		% get wall heights
		h1 = fp.edge_floors(wi);
		h2 = fp.edge_ceilings(wi);

		% get height of window
		wh = (h2 - h1) * (windows.max_vertical(i) ...
					- windows.min_vertical(i));
	
		% add to total area
		window_area = window_area + (ww*wh);
	end

	% compute the ratio
	r = window_area / wall_area;
end
