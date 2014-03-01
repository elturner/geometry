function oct = read_oct(filename)
	% oct = read_oct(filename)
	%
	%	Parses the specified .oct file into a tree structure
	%
	% arguments:
	%
	%	filename -	Path to .oct file
	%
	% output:
	%
	%	oct -	The oct tree structure, which is a list of the
	%		nodes in the tree.  Each node in this list
	%		has the following structure:
	%
	%			center - [x;y;z] center of node
	%			halfwidth - The half-width of the node
	%			hasdata - boolean indicates if data valid
	%			data - data structure of node, only valid
	%			       if hasdata == true
	%			children - length 8 list of child indices.
	%			           negative index means no child.
	%

	% open file for reading (binary)
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['could not open ', filename]);
	end

	% read in magic number 
	M = fread(fid, 8, 'schar')';
	if(any(M ~= ['octfile',0]))
		fclose(fid);
		error(['Not a valid octfile: ', filename]);
	end

	% read in max depth
	max_depth = fread(fid, 1, 'uint32');

	% read in nodes recursively
	nodes_read = 0;
	oct = struct('center', cell(0), ...
	             'halfwidth', cell(0), ...
	             'hasdata', cell(0), ...
	             'data', cell(0), ...
	             'children', cell(0));
	oct = read_nodes(fid, oct);

	% clean up
	fclose(fid);
end

function oct = read_nodes(fid, oct)
	% oct = read_nodes(fid, oct)
	%
	%	Reads octree nodes recursively in Depth-first manner
	%

	% read in current node
	i = length(oct) + 1;
	oct(i).center = fread(fid, 3, 'double');
	oct(i).halfwidth = fread(fid, 1, 'double');
	oct(i).hasdata = fread(fid, 1, 'schar');
	
	% read in data if available
	if(oct(i).hasdata)
		% populate data from file
		oct(i).data.count = fread(fid, 1, 'uint32');
		oct(i).data.prob_sum = fread(fid, 1, 'double');
		oct(i).data.prob_sum_sq = fread(fid, 1, 'double');
		oct(i).data.corner_sum = fread(fid, 1, 'double');
		oct(i).data.planar_sum = fread(fid, 1, 'double');
		oct(i).data.fp_room = fread(fid, 1, 'int32');
	else
		% populate data structure with garbage
		oct(i).data.count = -1;
		oct(i).data.prob_sum = -1;
		oct(i).data.prob_sum_sq = -1;
		oct(i).data.corner_sum = -1;
		oct(i).data.planar_sum = -1;
		oct(i).data.fp_room = -1;
	end

	% read in children recursively
	oct(i).children = -1 * ones(8,1);
	for j = 1:8
		% check if j'th child is defined
		haschild = fread(fid, 1, 'schar');
		if(haschild)
			% read in this child into the same structure
			oct(i).children(j) = length(oct)+1;
			oct = read_nodes(fid, oct);
		end
	end
end
