function [planar_probs, corner_probs] = compute_shape_probs(scan, dist, ang)
	% [planar_probs, corner_probs] = compute_shape_probs(scan,dist,ang)
	%
	%	Will compute the probability that each point in the
	%	given fss scan is planar or a corner.  This is done
	%	by attempting to fit one or two lines respectively.
	%
	% arguments:
	%
	%	scan -	A single fss scan
	%	dist -	The distance away from each point to analyze 
	%		neighbors
	%	ang -	The angular spacing between successive scan points
	%
	% output:
	%
	%	planar_probs -	The list of planar probabilities for
	%			each scan point
	%	corner_probs -	The list of corner probabilities for
	%			each scan point
	%

	% get properties of scan
	N = size(scan.pts, 2); % number of scan points
	
	% prepare output lists
	planar_probs = zeros(1, N);
	corner_probs = zeros(1, N);
	
	% iterate over the scan points
	for i = 1:N
		
		% look at neighborhood around this point
		r = norm(scan.pts(:,i)); % distance of point to scanner
		m = ceil(atan(dist/r)/ang); % maximum number of neighbors
		                            % that can possibly be in
		                            % ball of size 'dist' around
		                            % this point
		neigh_inds = [max(1,i-m):min(N,i+m)]; % index of neighbors
		inball = (sum((scan.pts(:,neigh_inds) ...
				- scan.pts(:,i)*ones(1, ...
				length(neigh_inds))).^2, 1) <= dist*dist);
		neigh_inds = neigh_inds(inball); % neighbors in range

		% now attempt to compute the probabilities for various
		% shape models
		planar_probs(i) = probability_of_planar(scan,neigh_inds);
		corner_probs(i) = probability_of_corner(scan,i,neigh_inds);
	end
end

function [p, line_model] = probability_of_planar(scan, neigh_inds)
	% [p, line_model] = probability_of_planar(scan, neigh_inds)
	%
	% 	Computes the probability that point #i is part of
	%	a plane, given the neighborhood of points around it
	%

	% check validity of input
	if(length(neigh_inds) < 2)
		% not enough samples to fit a line
		p = 0;
		line_model = struct();
	end

	% fit a line with pca
	line_model = line_fit_pca(scan.pts(:,neigh_inds));

	% compare the fit errors of this line with the distributions of
	% each point.

	% e represents the average distance of a neighbor point to the line,
	% normalized based on the uncertainties of the neighor points
	s = max(scan.stddev(neigh_inds), scan.width(neigh_inds));
	e = mean(line_model.err ./ s);

	% if we treat e as a sample from a unit gaussian, then 
	% we can express the probability of the line fit as the
	% probability mass greater than e (that is, if the points are
	% likely to be at least as far out as the line, then the line
	% is a good fit)
	%
	%                               |                                
	%                             --|--                               
	%                            /  |  \                             
	%                          _/   |   \_                           
	%              _______-----#|   |   |#-----_____________         
	%              #############|   |   |###################         
	% --------------------------------------------------------------
	%                          -e       e   
	%
	% p = 2 * cdf(-e) = erf(-e)+1
	p = erf(-e)+1;
end

function p = probability_of_corner(scan, i, neigh_inds)
	% p = probability_of_corner(scan, i, neigh_inds)
	%
	%	Computes the probability that point #i is part of
	%	a corner, given the neighborhood of points.  This
	%	will attempt to fit two lines to the neighbor points,
	%	where the lines intersect at the i'th point.
	%

	% split the neighbors into two groups	
	j = find(neigh_inds == i);
	neighs_left  = neigh_inds(1:j);
	neighs_right = neigh_inds(j:end);

	% fit a line to each group individually
	[p_left, line_left]   = probability_of_planar(scan,neighs_left);
	[p_right, line_right] = probability_of_planar(scan,neighs_right);

	% this is only a corner if both directions are lines and
	% if those lines are not parallel
	p = p_left*p_right*norm(cross(line_left.dir, line_right.dir));
end
