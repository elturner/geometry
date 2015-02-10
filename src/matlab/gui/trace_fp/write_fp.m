function [] = write_fp(filename, fp)
	% WRITE_FP(filename, fp)
	%
	%	Given a floorplan structure, will write it to
	%	the specified .fp file.
	%
	%	The .fp file format is used to represent floorplans
	%
	%	See also: 
	%		fp = READ_FP(filename)
	%	
	% arguments:
	%
	%	filename -	Where to write the file
	%	fp -		The fp structure to export
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Last modified on December 11, 2014
	%

	% prepare to write to file
	fid = fopen(filename, 'w');
	if(fid < 0)
		error(['Unable to write to file: ', filename]);
	end

	% check if misformated
	if(length(fp.room_floors) ~= length(fp.room_ceilings) ...
			|| length(fp.room_floors) ~= fp.num_rooms)
		error(['The fp struct is nonconsistent about ', ...
			'number of rooms']);
	end

	% write header to file
	fprintf(fid, '%f\n', fp.res);
	fprintf(fid, '%d\n', size(fp.verts,1));
	fprintf(fid, '%d\n', size(fp.tris,1));
	fprintf(fid, '%d\n', fp.num_rooms);

	% write the vertices to file
	for i = 1:size(fp.verts,1)
		fprintf(fid, '%f %f\n', fp.verts(i,1), fp.verts(i,2));
	end

	% write the triangles to file
	for i = 1:size(fp.tris,1)
		fprintf(fid, '%d %d %d\n', ...
			fp.tris(i,1)-1, fp.tris(i,2)-1, fp.tris(i,3)-1);
	end

	% write the rooms to file
	for i = 1:fp.num_rooms
		
		% write out the heights for this room
		fprintf(fid, '%f %f', ...
			fp.room_floors(i), fp.room_ceilings(i));

		% find all the triangles in this room
		T = find(fp.room_inds == i) - 1;
	
		% export the triangles
		fprintf(fid, ' %d', length(T));
		fprintf(fid, ' %d', T);
		fprintf(fid, '\n');
	end

	% clean up
	fclose(fid);
end
