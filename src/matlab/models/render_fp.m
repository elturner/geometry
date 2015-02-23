function [] = render_fp(floorplan, color_by_room, c, labels)
	% render_fp(floorplan)
	%
	%	Renders this floor plan on the current figure
	%
	% arguments:
	%
	%	floorplan -	The struct to render
	%
	%	color_by_room -	OPTIONAL. If true, will color each room
	%			separately. Default is false.  If this
	%			is an array of size NUM_ROOMS, then will
	%			use these colors for each room.
	%
	%	c -		OPTIONAL. Default color to use. Can specify
	%			[r g b] where each component is in [0,1],
	%			or can also specify optional [r g b alpha]
	%	
	%	labels -	OPTIONAL.  If true, will write room #'s.
	%			Default is false.
	%

	hold all;
	set(gcf, 'renderer', 'opengl');
	axis equal;
	axis off;

	% check arguments
	if(~exist('color_by_room', 'var') || isempty(color_by_room))
		color_by_room = false;
	end
	if(~exist('c', 'var') || isempty(c))
		c = [0.8 0.8 1 1];
	end
	if(length(c) < 4)
		c = [c, ones(1,4-length(c))];
	end
	if(~exist('labels', 'var') || isempty(labels))
		labels = false;
	end

	% make a color for each room
	if(numel(color_by_room) == 1)
		% treat color_by_room as a boolean, make up random colors
		if(color_by_room)
			colors = 0.25 + 0.5*rand(floorplan.num_rooms, 3);
		else
			colors = ones(floorplan.num_rooms, 1) * c(1:3);
		end
	elseif(numel(color_by_room) == floorplan.num_rooms)
		% treat color_by_room as a set of color values
		colors = colorme(color_by_room);
	else
		% unknown input
		error('unable to parse input value of color_by_room');
	end

	% plot triangles
	if(c(4) ~= 0)
		for i = 1:min(floorplan.num_tris, size(floorplan.tris,1))
		
			% check if valid room
			if(floorplan.room_inds(i) == 0)
				roomcol = [1 0 0];
				fprintf('Triangle #%d has no room\n', i);
			else
				roomcol = colors(floorplan.room_inds(i),:);
			end
			
			% draw it
			fill(floorplan.verts(floorplan.tris(i,:),1), ...
				floorplan.verts(floorplan.tris(i,:),2), ...
				roomcol, ...
				'FaceAlpha', c(4), ...
				'EdgeColor', 'none');
		end
	end

	% print room indices
	if(labels)
		for i = 1:floorplan.num_rooms
			ts = find(floorplan.room_inds == i);
			vs = floorplan.tris(ts,:);
			x_avg = mean(floorplan.verts(vs(:),1));
			y_avg = mean(floorplan.verts(vs(:),2));
			name = num2str(i-1);
			text(x_avg, y_avg, name);
		end
	end

	% plot edges
	line([floorplan.verts(floorplan.edges(:,1),1)' ; ...
		floorplan.verts(floorplan.edges(:,2),1)'], ...
		[floorplan.verts(floorplan.edges(:,1),2)' ; ...
		floorplan.verts(floorplan.edges(:,2),2)'], ...
		'LineWidth', 2, 'color', 'black');
end

function colors = colorme(array)
	% colors = colorme(array)
	%
	%	Given a set of N values as an array of scalars, 
	%	this function will return a Nx3 matrix representing
	%	a color for each given value.
	%
	% arguments:
	%
	%	array -		Either a row or column vector of scalars
	%
	% output:
	%
	%	colors -	The colors corresponding with each input
	%			value
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	Written January 12, 2015
	%

	% get bounds
	N = numel(array);
	minval = min(array);
	maxval = max(array);
	res = 256;

	% make a JETMAP array
	J = jet(res);

	% get indices for each input value
	I = 1 + round( (res-1) .* (array - minval) ./ (maxval - minval) );

	% create colors
	colors = J(I,:);
end
