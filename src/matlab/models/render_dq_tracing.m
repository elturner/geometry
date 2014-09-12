function [] = render_dq_tracing(dqfile, madfile, pose_inds)
	% RENDER_DQ_TRACING(dqfile, madfile)
	%
	%	Will render the specified wall samples and path,
	%	as well as any line-of-sight information represented
	%	in the dq.
	%
	%	NOTE: ONLY DO THIS FOR VERY SIMPLE MODELS
	%
	% arguments:
	%
	%	dqfile -	The location of the wall samples on disk
	%	madfile -	The location of the path on disk
	%	pose_inds -	OPTIONAL.  Only show the given indices.
	%			If not specified, shows all poses.
	%
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Last modified September 11, 2014
	%

	% read in the wall samples
	dq = readMapData(dqfile);

	% read in poses
	poses = readMAD(madfile);

	% prepare figure
	hold on;
	axis equal;
	set(gcf, 'renderer', 'opengl');
	
	% iterate over the wall samples, drawing them
	lines_X = zeros(2, 0);
	lines_Y = zeros(2, 0);
	for i = 1:size(dq.pos, 2)

		% draw this wall sample point
		if(dq.numPoints(i) > 0)
			plot(dq.pos(1,i), dq.pos(2,i), 'b.');
		else
			plot(dq.pos(1,i), dq.pos(2,i), 'rx');
		end

		% iterate over the poses that saw this sample
		for j = 1:length(dq.poseIdx{i})

			% get pose position
			ind = dq.poseIdx{i}(j) + 1; % indexed from 0
			p = poses(1:2, ind);

			% filter by given pose inds
			if(exist('pose_inds', 'var') ...
				&& ~isempty(pose_inds) ...
					&& ~any(pose_inds == ind))
				continue;
			end

			% record a line from this pose to the sample
			lines_X(:,end+1) = [dq.pos(1,i); p(1)];
			lines_Y(:,end+1) = [dq.pos(2,i); p(2)];
		end
	end

	% draw all lines at once
	subsample = 10;
	line(lines_X(:,1:subsample:end), lines_Y(:,1:subsample:end));
end
