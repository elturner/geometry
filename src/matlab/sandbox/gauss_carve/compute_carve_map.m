function p = compute_carve_map(scan, x, xsize)
	% p = compute_carve_map(scan, x, xsize)
	%
	%	Shows the carving map generated from a scan line.
	%	At each position in space, this map will report what
	%	the likelihood of that position being interior is.
	%
	%	A value of 1.0 indicates high certainty that the location
	%	is interior.  A value of 0.0 indicates high certainty
	%	that a location is exterior.  A value of 0.5 indicates 
	%	no certainty.
	%
	% arguments:
	%
	%	scan - The statistics of the originating scan
	%	x - The location to sample
	%	xsize - The characteristic length of the area at x
	%
	% output:
	%
	%	p - The value of the carve map at location x
	%

	% we want to compute a marginalization of the line pdf in
	% cylindrical coordinates.  First, find the 1D gaussian
	% distribution along the length of the ray for both end points
	
	% 1D mean of marginal from intersection with sensor's distrib
	ms_dist = dot(scan.Es - x, scan.eig_s) / scan.eig_s_dot;
	ss = scan.varu_s; % variance of this intersected marginal 

	% 1D mean of marginal from intersection wtih scan point's distrib
	mp_dist = dot(scan.Ep - x, scan.eig_p) / scan.eig_p_dot;
	sp = scan.varu_p; % variance of this intersected marginal

	% get the probability value that x is after the sensor position
	% AND that x is before the end of the ray.
	%
	% Note that 'x' is at distance zero in the 1D coordinate along
	% this line.
	p_forward = gauss_cdf(ms_dist, ss, 0);
	p_inrange = (1 - gauss_cdf(mp_dist, sp, 0));

	% get the lateral position of this point away from the mean ray
	% of this scan.  Also determine the lateral variance of the scan
	% at this position
	f = min(1, max(0, -ms_dist / (mp_dist - ms_dist)));
	E = f*scan.Ep + (1-f)*scan.Es; % blended mean
	C = f*scan.Cp + (1-f)*scan.Cs; % blended covariance
	lat = (x - E); % lateral position of X from blended mean
	latdist = norm(lat); % lateral distance
	lat = lat / latdist; % unit direction of lateral position
	varlat = lat' * C * lat; % lateral variance

	% get the probability value that position x is laterally intersected
	% by the ray, given the size of x in the lateral direction
	p_lat = gauss_pdf(0, varlat, latdist) * xsize;

	% combine into full probibility
	
	% the a priori value is what the output approaches as our
	% confidence goes to zero
	interior_val = 1.0;
	toofar_val   = 0.0;
	a_priori_val = 0.5;

	% the output is a bournoulli expected value, which we can estimate
	% by a weighted average of the different states, using the
	% probabilities calculated above as weights
	p =	  (p_forward*p_lat      *p_inrange)     * interior_val ...
		+ (p_lat*p_forward      *(1-p_inrange)) * toofar_val ...
		+ ((1-(p_lat*p_forward))*p_inrange)     * a_priori_val ...
		+ ((1-(p_lat*p_forward))*(1-p_inrange)) * a_priori_val;
end

function y = gauss_cdf(mu, var, x)
	% y = gauss_cdf(mu, var, x)
	%
	%	Given a 1D gaussian with mean mu and variance var,
	%	will report the value of the cdf at position x
	%

	y = 0.5*(1 + erf( (x - mu) / sqrt(2*var) ));
end

function y = gauss_pdf(mu, var, x)
	y = (1/sqrt(2*pi*var)) * exp( -(x-mu)^2/(2*var) );
end
