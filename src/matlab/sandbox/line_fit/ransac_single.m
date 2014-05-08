function line_model = ransac_single(P_free, S_free)
	% line_model = ransac_single(P_free, S_free)
	%
	%	Given the set of unfitted points in a scan, will
	%	perform ransac to find the best-fit line for these
	%	points.
	%
	% arguments:
	%
	%	P_free -	A 3xN list of unfitted points to test
	%	S_free -	A 1xN list of standard deviations of each
	%			point's position
	%
	
	% ransac parameters
	num_iters = 1000;
	std_thresh = 5;	% number of std. devs. of a point 
			% that a line has to be away for the 
			% point to be an inlier of that line
	
	% prepare models of lines for comparison
	N = size(P_free,2);
	best_model = struct('dir',[],'p',[], ...
		'err',[],'inliers',false(1,N)); 
	best_count = 0;
	
	% run iterations to find best model
	for it = 1:num_iters

		% pick two points at random
		I = randperm(N,2);

		% get the model for this line
		M = line_fit_pca(P_free(:,I));

		% get distance of all the points from this model
		M.err = get_distance_from_line(P_free, M);

		% find inliers for this model based on known statistics
		inliers = ((std_thresh*S_free) > M.err);

		% compare this model to the best model so far 
		if(sum(inliers) > best_count)
		
			% this model is the best model so far
			% regenerate model based on inliers
			M = line_fit_pca(P_free(:,inliers));
		
			% store in best model
			best_model.dir = M.dir;
			best_model.p = M.p;
			best_model.err = M.err;
			best_model.inliers = inliers;
			best_count = sum(inliers);
		end
	end

	% return the best model
	line_model = best_model;
end

function dists = get_distance_from_line(P, line)
	% Computes the distances of each point in P to the line 
	% modeled in line
	%
	
	% get distance of all the points from this model
	N = size(P,2);
	dists = zeros(1,N);
	E = P - line.p*ones(1,N); % displacement of points
	for i = 1:N
		% distance of i'th point to line
		dists(i) = norm(P(:,i) - line.dir*dot(P(:,i), line.dir));
	end
end
