function [] = render_fp_list(dirname, dq)
	% render_fp_list(dirname, dq)
	%
	%	Will render each floorplan file listed
	%	in the given directory, in order.  Press
	%	spacebar to move to next floorplan.
	%
	% arguments:
	%
	%	dirname -	Directory containing floorplan (.fp) files
	%	dq -		OPTIONAL.  The wall sample map data to
	%			plot on top of floorplan.
	%

	% prepare figure
	figure;
	fixtheaxis = false;

	% iterate over files
	D = dir(dirname);
	for i = 1:length(D)

		% ignore files that are not floorplans
		d = D(i);
		n = length(d.name);
		if(d.isdir || n < 3)
			continue;
		end
		if(~strcmp(d.name(n-2:end),'.fp'))
			continue;
		end

		% import floorplan from disk
		f = fullfile(dirname, d.name);
		fp = read_fp(f);
		a = axis;
		clf;
		render_fp(fp);

		% also plot wall samples if available
		if(exist('dq', 'var'))
			render_map_data(dq);
		end

		% keep same view between frames
		if(fixtheaxis)
			axis(a);
		end
		fixtheaxis = true;
		title(['Floorplan: ', texlabel(d.name, 'literal')], ...
				'FontSize', 24);
		pause;
	end
end
