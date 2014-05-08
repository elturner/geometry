function [] = render_line_model(M, X)
	% render_line_model(M, X)
	%
	%	Will render the specified line model and points in
	%	the current figure
	%
	% arguments:
	%
	%	M -	The line model generated from ransac_single()
	%	X -	The input points (optional)
	% 

	% prepare figure
	hold all;
	axis equal;

	% plot the line
	s = 3000;
	plot3(M.p(1) + M.dir(1)*[-s s], ...
		M.p(2) + M.dir(2)*[-s s], ...
		M.p(3) + M.dir(3)*[-s s], 'r-');

	% plot points if available
	if(exist('X', 'var'))

		plot3(X(1,:), X(2,:), X(3,:), 'k.');

		% plot inliers
		plot3(X(1,M.inliers),X(2,M.inliers),X(3,M.inliers),'ro');
	end

end
