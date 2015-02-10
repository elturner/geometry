function ind = TraceFP_select(handles)
	% ind = TraceFP_select(handles)
	%
	%	Selects a single control point via the mouse
	%


	% if wall samples are defined, then need to ignore them
	if(handles.wall_samples_plot ~= 0)

		% if triangles defined, ignore them
		if(handles.triangles_plot ~= 0)
			% ignore both
			ind = selectdata('sel', 'br', ...
				'Ignore', [handles.wall_samples_plot, ...
				handles.triangles_plot]);

		else
			% ignore wall samples
			ind = selectdata('sel', 'br', ...
				'Ignore', handles.wall_samples_plot);
		end

	else
		% if triangles defined, ignore them
		if(handles.triangles_plot ~= 0)
			% ignore triangles
			ind = selectdata('sel', 'br', ...
				'Ignore', handles.triangles_plot);

		else
			% ignore wall samples
			ind = selectdata('sel', 'br');
		end
	end

	if(isempty(ind))
		fprintf('[TraceFP]\t\tno point selected\n');
		ind = -1;
		return; % do nothing
	elseif(numel(ind) > 1)
		fprintf(['[TraceFP]\t\tmultiple points selected, ', ...
				'only moving one\n']);
		ind = ind(1);
	end
end
