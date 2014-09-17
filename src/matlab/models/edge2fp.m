function [] = edge2fp(edgefile, fpfile)
	% edge2fp(edgefile, fpfile)
	%
	%	Will convert the set of walls described in the
	%	input .edge file to a volumetric floorplan represented
	%	in a .fp file.
	%
	% edge2fp({vertfile1, vertfile2, ...}, fpfile)
	%
	%	Will convert a set of vertices from the input .vert files
	%	to a .fp file.  The vertices are given as a cell array
	%	of .vert files.  Each .vert file is formatted as described
	%	below.
	%
	% arguments:
	%
	%	edgefile -	The input edge file to convert.  Each
	%			line in the file should represent a
	%			wall as a line-segment, by defining
	%			four values:
	%
	%			<start_x> <start_y> <end_x> <end_y>
	%
	%			It is assumed that the values are
	%			in world coordinates, in units of
	%			meters.
	%
	%	vertfile -	The vert file is formatted as follows:
	%
	%				<x1> <y1>
	%				<x2> <y2>
	%
	%			Each .vert file should represent one
	%			closed polygon in the environment.  So,
	%			for example, if the floorplan has a set
	%			of pillars, then each pillar should be its
	%			own .vert file.
	%
	%	fpfile -	The output .fp file to generate.  This
	%			file format is described in 
	%			docs/filetypes/fp_file_format.txt
	%
	%			The floorplan will be given some default
	%			values, such as all triangles being
	%			in the same room, and all vertices having
	%			the same default height.
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	on August 17, 2014
	%

	% check if given a single input file or a list of files
	if(iscell(edgefile))
		% load as a list of vertex files, where each file
		% only has <x> <y> per line, and defines a closed
		% polygon
		infile_list = edgefile;
		C = [];
		verts_x = [];
		verts_y = [];
		for i = 1:length(infile_list)
			
			% load current polygon
			vs = load(infile_list{i});

			% add constraints
			C = [C ; length(verts_x) ...
				+ [ [1:size(vs,1)]', ...
				[2:size(vs,1), 1]' ] ];
			
			% add vertices
			verts_x = [verts_x; vs(:,1)];
			verts_y = [verts_y; vs(:,2)];
		end

	else
		% import the wall line segments
		W = load(edgefile);

		% check format of input
		N = size(W,1);
		if(size(W,2) ~= 4)
			error('Input .edge file incorrectly formatted');
		end

		% represent the wall segments as vertices and edges
		verts_x = W(:,[1 3])'; verts_x = verts_x(:);
		verts_y = W(:,[2 4])'; verts_y = verts_y(:);
	
		% the segments themselves are constraints 
		% on the triangulation
		C = [ [1:(N-1)]', [2:N]' ]; % each edge is a constraint

	end

	% generate the triangulation
	% (This will throw a warning about duplicate vertices, which
	% can be ignored)
	orig_state = warning;
	warning('off','all');
	dt = delaunayTriangulation(verts_x, verts_y, C);
	warning(orig_state);

	% prepare to write triangulation out as a floorplan.  
	% This assumes some default values
	DEFAULT_RESOLUTION     = 0.05;
	DEFAULT_FLOOR_HEIGHT   = 0.0;
	DEFAULT_CEILING_HEIGHT = 0.0;

	% get attributes of triangulation
	num_verts = size(dt.Points,1);
	io = dt.isInterior()';
	num_tris = sum(io);

	% prepare file
	outfile = fopen(fpfile, 'w');
	if(outfile < 0)
		error('unable to open .fp file for writing');
	end

	% write header
	fprintf(outfile, '%f\n', DEFAULT_RESOLUTION); % resoltion in meters
	fprintf(outfile, '%d\n', num_verts); % number of vertices
	fprintf(outfile, '%d\n', num_tris); % num tris
	fprintf(outfile, '1\n'); % only one room

	% print out the points
	for i = 1:num_verts
		fprintf(outfile, '%f %f\n', dt.Points(i,1), dt.Points(i,2));
	end

	% print out the triangles.  The triangles index into
	% the list of vertices, starting at index 0
	for i = find(io)
		fprintf(outfile, '%d %d %d\n', ...
			dt.ConnectivityList(i,1)-1, ...
			dt.ConnectivityList(i,2)-1, ...
			dt.ConnectivityList(i,3)-1);
	end

	% print the rooms (there's only one)
	fprintf(outfile, '%f %f %d', ...
			DEFAULT_FLOOR_HEIGHT, ...
			DEFAULT_CEILING_HEIGHT, num_tris);
	fprintf(outfile, ' %d', [0:num_tris-1]); % all the triangles
	fprintf(outfile, '\n');

	% clean up
	fclose(outfile);
end
