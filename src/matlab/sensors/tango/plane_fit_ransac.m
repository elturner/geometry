function [plane,inliers,err] = plane_fit_ransac(P)
	% [plane,err] = PLANE_FIT_RANSAC(P)
	%
	%	Given a set of 3D points, will find the best-fit
	%	plane to the points using Random Sample Consensus
	%	(RANSAC).
	%
	% arguments:
	%
	%	P -	One scan frame, which should be a 3xN matrix,
	%		where each column is [x;y;z]
	%
	% output:
	%
	%	plane -	A structure with the following fields
	%
	%			offset -	a 3D point on the plane
	%			norm -		Normal vector of plane
	%		
	%	inliers - a 1xN boolean array, indicating which points
	%		  are inliers.
	%
	%	err -	List of signed distance errors for each 
	%		input point
	%

	% properties of input
	N = size(P,2);

	% parameters for ransac
	num_iters = 100; % number of iterations
	thresh = 0.05; % fitting threshold for points to plane (units: m)
	sufficient_data = 0.5 * N;
		% if this number of points are fit, then model is
		% good enough to test

	% prepare fields
	plane_best = zeros(3,1); % best plane model so far
	inliers_best = zeros(1,N);
	f_best = 0; % best fit value (number of inliers)

	% run iterations
	for iter = 1:num_iters

		% select three points randomly
		ii = randperm(N, 3);
		ii_P = P(:,ii);

		% find the plane defined by these points
		ii_plane = plane_fit(ii_P);

		% get the inliers to this plane based on fitting threshold
		e = dot(P - ii_plane.offset*ones(1,N), ...
		        ii_plane.norm*ones(1,N));
		inliers = find( abs(e) < thresh );

		% check if we have a sufficient number of inliers
		if(length(inliers) < sufficient_data)
			continue;
		end

		% fit model to all inliers
		in_P = P(:,inliers);
		in_plane = plane_fit(in_P);

		% recompute number of inliers
		e = dot(P - in_plane.offset*ones(1,N), ...
		        in_plane.norm*ones(1,N));
		f = sum(abs(e) < thresh);

		% compare to recorded best model
		if(f_best < f)
			f_best = f;
			plane_best = in_plane;
			inliers_best = inliers;
		end
	end

	% export plane and error
	plane = plane_best;
	inliers = inliers_best;
	
	% compute error values for each point if requested by user
	if(nargout >= 2)

		% scale point vectors along their look direction to
		% be on the best-fit plane (assuming vector originates
		% at the origin)
		s = dot(plane.norm, plane.offset) ...
			./ dot(plane.norm*ones(1,N), P, 1);

		% generate error list by converting scale factor to
		% offset distance by multiplying by magnitude
		err = (s - 1) .* sqrt(dot(P,P,1));
	end
end
