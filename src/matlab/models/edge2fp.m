function [] = edge2fp(edgefile, fpfile)
	% edge2fp(edgefile, fpfile)
	%
	%	Will convert the set of walls described in the
	%	input .edge file to a volumetric floorplan represented
	%	in a .fp file.
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

	% the segments themselves are constraints on the triangulation
	C = [ [1:(N-1)]', [2:N]' ]; % each edge is a constraint

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
	num_tris = size(dt.ConnectivityList,1);

	% prepare file
	outfile = fopen(fpfile, 'w');
	if(outfile < 0)
		error('unable to open .fp file for writing');
	end

	% write header
	fprintf(outfile, '%f\n', DEFAULT_RESOLUTION); % resoltion in meters
	fprintf(outfile, '%f\n', num_verts); % number of vertices
	fprintf(outfile, '%f\n', num_tris); % num tris
	fprintf(outfile, '1\n'); % only one room

	% print out the points
	for i = 1:num_verts
		fprintf(outfile, '%f %f\n', dt.Points(i,1), dt.Points(i,2));
	end

	% print out the triangles.  The triangles index into
	% the list of vertices, starting at index 0
	for i = 1:num_tris
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
