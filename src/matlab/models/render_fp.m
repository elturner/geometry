function [] = render_fp(floorplan, color_by_room, c)
	% render_fp(floorplan)
	%
	%	Renders this floor plan on the current figure
	%
	% arguments:
	%
	%	floorplan -	The struct to render
	%
	%	color_by_room -	OPTIONAL. If true, will color each room
	%			separately. Default is false.
	%

	hold all;
	axis equal;
	axis off;

	% check arguments
	if(~exist('color_by_room', 'var'))
		color_by_room = false;
	end
	if(~exist('c', 'var'))
		c = [0.8 0.8 1];
	end

	% make a color for each room
	if(color_by_room)
		colors = 0.25 + 0.5*rand(floorplan.num_rooms, 3);
	else
		colors = ones(floorplan.num_rooms, 1) * c;
	end

	% plot triangles
	for i = 1:floorplan.num_tris
		fill(floorplan.verts(floorplan.tris(i,:),1), ...
			floorplan.verts(floorplan.tris(i,:),2), ...
			colors(floorplan.room_inds(i),:), ...
			'EdgeColor', 'none');
	end

	% plot edges
	line([floorplan.verts(floorplan.edges(:,1),1)' ; ...
		floorplan.verts(floorplan.edges(:,2),1)'], ...
		[floorplan.verts(floorplan.edges(:,1),2)' ; ...
		floorplan.verts(floorplan.edges(:,2),2)'], ...
		'LineWidth', 2, 'color', 'black');
end
