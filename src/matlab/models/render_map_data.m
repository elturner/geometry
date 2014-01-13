function [] = render_map_data(map_data, showheights)
	% render_map_data(map_data)
	%
	%	Given a map from a *.dq file, will
	%	render to current figure.
	%

	hold all;
	axis equal;
	set(gcf, 'renderer', 'opengl');

	% plot points
	N = size(map_data.pos, 2);
	if(~exist('showheights', 'var'))
		
		% plot 2D position
		plot(map_data.pos(1,:), map_data.pos(2,:), 'b.');
		
	else
		% plot 2.5 D positions and ranges
		plot3(map_data.pos(1,:), map_data.pos(2,:), ...
					map_data.heightRange(1,:), 'b.');
		plot3(map_data.pos(1,:), map_data.pos(2,:), ...
					map_data.heightRange(2,:), 'r.');

		%line([1;1]*map_data.pos(1,:), ...
		%	[1;1]*map_data.pos(2,:), ...
		%	map_data.heightRange, ...
		%	'color', 'blue');
	end
end
