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

	% read in header info
	max_depth = fread(fid, 1, 'uint32');
	num_nodes = fread(fid, 1, 'uint32');

	% read in nodes recursively
	nodes_read = 0;
	oct = struct('center', cell(num_nodes,1), ...
	             'halfwidth', cell(num_nodes,1), ...
	             'hasdata', cell(num_nodes,1), ...
	             'data', cell(num_nodes,1), ...
	             'isleaf', cell(num_nodes,1), ...
		     'children', cell(num_nodes,1));
	oct = read_nodes(fid, oct, 0);

	% clean up
	fclose(fid);
end

function [oct,n] = read_nodes(fid, oct, n)
	% oct = read_nodes(fid, oct, n)
	%
	%	Reads octree nodes recursively in Depth-first manner
	%
	% arguments:
	%
	%	fid -	The file to read from
	%	oct -	The struct array to read to
	%	n -	How many nodes have already been read in
	% 
	% output:
	%
	%	oct -	The populated struct
	%	n -	How many nodes have been filled
	%

	% show progress to user
	if(mod(n, 1000) == 0)
		disp(['read in ', num2str(n), '/', ...
			num2str(length(oct)), ' nodes']);
	end

	% read in current node
	n = n + 1; % filling another node
	i = n; % index of latest node
	oct(i).center = fread(fid, 3, 'double');
	oct(i).halfwidth = fread(fid, 1, 'double');
	oct(i).hasdata = fread(fid, 1, 'schar');
	
	% read in data if available
	if(oct(i).hasdata)
		% populate data from file
		oct(i).data.count = fread(fid, 1, 'uint32');
		oct(i).data.prob_sum = fread(fid, 1, 'double');
		oct(i).data.prob_sum_sq = fread(fid, 1, 'double');
		oct(i).data.surface_sum = fread(fid, 1, 'double');
		oct(i).data.corner_sum = fread(fid, 1, 'double');
		oct(i).data.planar_sum = fread(fid, 1, 'double');
		oct(i).data.fp_room = fread(fid, 1, 'int32');
	else
		% populate data structure with garbage
		oct(i).data.count = -1;
		oct(i).data.prob_sum = -1;
		oct(i).data.prob_sum_sq = -1;
		oct(i).data.surface_sum = -1;
		oct(i).data.corner_sum = -1;
		oct(i).data.planar_sum = -1;
		oct(i).data.fp_room = -1;
	end

	% read in children recursively
	oct(i).children = -1 * ones(8,1);
	oct(i).isleaf = true;
	for j = 1:8
		% check if j'th child is defined
		haschild = fread(fid, 1, 'schar');
		if(haschild)
			% read in this child into the same structure
			oct(i).isleaf = false;
			oct(i).children(j) = n+1;
			[oct, n] = read_nodes(fid, oct, n);
		end
	end
end
