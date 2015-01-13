function windows = read_windows(windowsfile, fp)
	% windows = READ_WINDOWS(windowsfile, fp)
	%
	%	Will parse a .windows file into a struct
	%
	%	A .windows file is associated with a particular
	%	floorplan file, and defines windows on a subset
	%	of walls on that floorplan.
	%
	%	Note that in the .windows file, all indices are
	%	zero-indexed, but they are recomputed to be
	%	one-index when imported into this structure.
	%
	% arguments:
	%
	%	windowsfile -	Path to the .windows file on disk
	%	
	%	fp -		The floorplan structure associated
	%			with these windows.
	%
	% output:
	%
	%	windows -	The output struct, which contains
	%			the following fields:
	%
	%			num_windows -	Number of windows defined
	%			
	%			wall_inds -	The walls for each window,
	%					size Nx2, where each row
	%					is a pair of indices of
	%					vertices in the floorplan,
	%					indicating an edge.
	%			
	%			min_horizontal -
	%			min_vertical -
	%			max_horizontal -
	%			max_vertical -
	%					The relative dimensions
	%					of each window on its
	%					wall.  Size Nx1.  In
	%					range [0,1], as a fraction
	%					of wall size.
	%			
	%			verts2d -	Size Nx4.  Gives the x,y
	%					coordinates of the window
	%					edges in model space:
	%
	%					[x1, y1, x2, y2]
	%
	%					units: meters
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on January 13, 2015
	%

	% read in the windows file
	windat = load(windowsfile);
	N = size(windat, 1); % number of windows

	% check valid input
	if(size(windat, 2) ~= 6)
		error(['Invalid format of .windows file ', ...
			'(wrong number of columns): ', windowsfile]);
	end

	% prepare windows structure
	windows.num_windows    = N;
	windows.wall_inds      = 1 + windat(:, 1:2); % make one-indexed
	windows.min_horizontal = windat(:, 3);
	windows.min_vertical   = windat(:, 4);
	windows.max_horizontal = windat(:, 5);
	windows.max_vertical   = windat(:, 6);

	% check if floorplan is present
	if(~exist('fp', 'var') || isempty(fp))

		% no floorplan given
		warning(' --- No floorplan structure given --- ');
		windows.verts2d = [];
		return;
	end

	% initialize world-coordinate fields
	windows.verts2d = zeros(N,4);

	% compute the world-coord vertices from the floorplan for
	% each window
	for i = 1:N

		% check valid vertex indices
		I = windows.wall_inds(i,:);
		if(min(I) <= 0 || max(I) > fp.num_verts)
			error('Window #%d: invalid wall vertex indices!',i);
		end

		% get wall 2d vertex positions
		vs = fp.verts(I, :); % size 2x2
		tangent = vs(2,:) - vs(1,:);

		% get window positions
		w1 = vs(1,:) + (tangent .* windows.min_horizontal(i));
		w2 = vs(1,:) + (tangent .* windows.max_horizontal(i));
		windows.verts2d(i,:) = [w1(1) w1(2) w2(1) w2(2)];
	end

end
