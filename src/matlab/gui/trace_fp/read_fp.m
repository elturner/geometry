function floorplan = read_fp(filename)
	% floorplan = READ_FP(filename)
	%
	%	Reads the *.fp file specified, and stores
	%	the values in the output struct
	%
	% output:
	%
	%	The output geometry is expressed in meters.
	%	Output struct contains the following fields:
	%
	%	res -		Resolution of floor plan
	%
	%	num_verts - 	Number of vertices
	%
	%	verts -		Matrix of vertices in floor-plan.  
	%			Size <num_verts> x 2
	%	
	%	num_tris -	Number of triangles in floor-plan
	%
	%	tris -		Matrix of triangles, sized
	%			<num_tris> x 3
	%
	%	edges -		Matrix of edges in floor-plan.  Size Mx2
	%
	%	edge_floors -	The floor level for each edge. Size Mx1
	%
	%	edge_ceilings -	The ceiling level for each edge. Size Mx1
	%
	%	num_rooms -	Number of rooms in floor plan.
	%
	%	room_inds -	Room index for each triangle, sized
	%			<num_tris> x 1, each element in 
	%			[1,num_rooms]
	%
	%	room_floors -	The heights of the floor for each room,
	%			sized <num_rooms> x 1
	%
	%	room_ceilings -	The heights of the ceilings for each room,
	%			sized <num_rooms> x 1
	%

	% initialize struct
	floorplan = struct('res', 0, 'num_verts', 0, 'verts', cell(1), ...
			'num_tris', 0, 'tris', cell(1), ...
			'edges', cell(1), 'edge_floors', cell(1), ...
			'edge_ceilings', cell(1), ...
			'num_rooms', 0, 'room_inds', cell(1), ...
			'room_floors', cell(1), 'room_ceilings', cell(1));

	% read header (1x3 array)
	h = dlmread(filename, ' ', [0 0 3 0]);
	floorplan.res = h(1);
	floorplan.num_verts = h(2);
	floorplan.num_tris = h(3);
	floorplan.num_rooms = h(4);

	% read vertices
	floorplan.verts = dlmread(filename, ' ', [4 0 ...
					(4+floorplan.num_verts-1) 1]);

	% read triangles
	if(floorplan.num_tris > 0)
		offset = 4 + floorplan.num_verts;
		floorplan.tris = 1 + dlmread(filename, ' ', [offset 0 ...
					(offset + floorplan.num_tris-1) 2]);
		offset = offset + floorplan.num_tris;

		% compute edges
		TR = TriRep(floorplan.tris, floorplan.verts(:,1), ...
						floorplan.verts(:,2));
		floorplan.edges = freeBoundary(TR);
	else
		% no triangles defined
		floorplan.tris = zeros(0,3);
		floorplan.edges = [];
	end

	% check if any rooms are defined
	if(floorplan.num_rooms == 0)
		return;
	end

	% read heights
	h = dlmread(filename, ' ', [offset 0 ...
			 (offset + floorplan.num_rooms-1) 2]);
	floorplan.room_floors = h(:,1);
	floorplan.room_ceilings = h(:,2);
	tris_per_room = h(:,3);

	% initialize vertex and edge heights
	vert_floors =      -10000000*ones(size(floorplan.verts, 1), 1);
	vert_ceilings =     10000000*ones(size(floorplan.verts, 1), 1);
	floorplan.edge_floors =   zeros(size(floorplan.edges, 1), 1);
	floorplan.edge_ceilings = zeros(size(floorplan.edges, 1), 1);

	% read each room's triangles
	floorplan.room_inds = zeros(floorplan.num_tris, 1);
	for i = 1:floorplan.num_rooms
	
		% get the list of triangle for this room
		T = 1 + dlmread(filename, ' ', [(offset+i-1) 3 ...
			 (offset+i-1) (3+tris_per_room(i)-1)]);
		
		% label these triangles
		floorplan.room_inds(T) = i;
	
		% get vertices for current triangles
		vi = floorplan.tris(T, :);
		vi = unique(vi(:));

		% update vertex heights
		vert_floors(vi(vert_floors(vi) ...
			< floorplan.room_floors(i))) ...
				= floorplan.room_floors(i);
		vert_ceilings(vi(vert_ceilings(vi) ...
			> floorplan.room_ceilings(i))) ...
				= floorplan.room_ceilings(i);
	end

	% compute edge heights
	for i = 1:size(floorplan.edges, 1)
		floorplan.edge_floors(i) ...
			= min(vert_floors(floorplan.edges(i,:)));
		floorplan.edge_ceilings(i) ...
			= max(vert_ceilings(floorplan.edges(i,:)));
	end
end
