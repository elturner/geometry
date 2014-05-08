function [line_models, P_line, P_corner] = line_fit(scan)
	% [line_models, P_line, P_corner] = line_fit(scan)
	%
	%	Will perform line fitting on the points of the specified
	%	scan.  Will export models for all fit lines, as well as
	%	labels for each point for the likelihood that it belongs
	%	to a line and the likelihood that it belongs to a corner
	%	(two or more lines).
	%
	% arguments:
	%
	%	scan -	A single fss scan frame
	%
	% output:
	%
	%	line_models -	A struct list of line models
	%
	%	P_line -	An array of probabilities associated with
	%			each input point, specifying how likely
	%			it is to be part of a line
	%
	%	P_corner -	An array of probabilities associated with
	%			each input point, specifying how likely
	%			it is to be the location of a corner.
	%

	% prepare input
	N           = size(scan.pts, 2); % total number of points
	S           = max(scan.stddev, scan.width); % uncertainty for each
	is_free     = true(1, N); % whether it has been fit to a model

	% initialize output
	line_models = struct('dir',cell(0),'p',cell(0), ...
				'err',cell(0),'inliers',cell(0));
	P_line      = zeros(1,N);
	P_corner    = zeros(1,N);


	% run ransac until every point has been fitted to a line
	while(sum(is_free) > 1)

		% perform ransac to find the best-fit line on all
		% the free points
		M = ransac_single(scan.pts(:,is_free), S(is_free));
		if(~any(M.inliers))
			continue;
		end

		% add this to our list of line models
		line_models(end+1).dir = M.dir;
		line_models(end).p = M.p;
		line_models(end).err = M.err;
		
		% the list of inliers reported by ransac_single is
		% a reference to the indices of the input list, but
		% we need to update this to reference the indices of
		% the full list of points
		il = false(N,1);
		il(is_free) = M.inliers;
		line_models(end).inliers = il;

		% update the list of free points
		is_free(il) = false;
	end

	% TODO
end
