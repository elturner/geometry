function line_model = line_fit_pca(pts)
	% line_model = line_fit_pca(pts)
	%
	%	Will perform PCA analysis to fit a line to the given
	%	set of points.
	%
	% arguments:
	%
	%	pts -	A 3xN matrix representing N points
	%
	% output:
	%
	%	line_model -	A struct representing the best-fit line
	%			for these points.  It contains the following
	%			subfields:
	%
	%				dir -	Unit vector of line
	%				p -	Point on the line
	%				err -	A 1xN list of distances
	%					of each input point.
	%

	% get the statistics
	N = size(pts, 2); % number of points
	m = mean(pts, 2); % mean position
	P = pts - m*ones(1, N); % unbiased points
	C = P*P'; % covariance matrix of points

	% perform PCA
	[V,D] = eig(C); % eigenvalue decomposition
	[junk,i] = max(diag(D));

	% populate model
	line_model.dir = V(:,i); % eigenvector corresponding to largest val
	line_model.p = m; % line positioned at mean point
	line_model.err = zeros(1,N); % distance of each point from line

	% populate error model
	for i = 1:N
		line_model.err(i) = norm(P(:,i) - ...
				line_model.dir*dot(P(:,i),line_model.dir));
	end

end
