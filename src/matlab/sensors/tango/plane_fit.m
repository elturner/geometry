function [plane,err] = plane_fit(frame)
	% [plane,err] = PLANE_FIT(frame)
	%
	%	Given a set of 3D points, will find the best-fit
	%	plane to the points using Principal Component Analysis
	%	(PCA).
	%
	% arguments:
	%
	%	scan -	One scan frame, which should be a 3xN matrix
	%		where each column is a point [x;y;z]
	%
	% output:
	%
	%	plane -	A structure with the following fields
	%
	%			offset -	a 3D point on the plane
	%			norm -		Normal vector of plane
	%			
	%	err -	List of signed distance errors for each 
	%		input point
	%

	% get the mean position of the points
	N = size(frame,2);
	plane.offset = mean( frame, 2 );
	
	% construct zero-mean version of pointcloud (Nx3)
	pc = (frame - plane.offset*ones(1,N))';
	
	% set up a covariance matrix of these points and perform eigen
	% decomposition
	C = cov(pc);
	[V,D] = eig(C);

	% the smallest eigenvalue corresponds with the plane's normal
	plane.norm = V(:,1);

	% verify normal is pointed towards origin
	if(dot(plane.norm, plane.offset) > 0)
		plane.norm = -1 * plane.norm;
	end

	% compute error values for each point if requested by user
	if(nargout >= 2)

		% scale point vectors along their look direction to
		% be on the best-fit plane (assuming vector originates
		% at the origin)
		s = dot(plane.norm, plane.offset) ...
			./ dot(plane.norm*ones(1,N), frame, 1);

		% generate error list by converting scale factor to
		% offset distance by multiplying by magnitude
		err = (s - 1) .* sqrt(dot(frame,frame,1));
	end
end
