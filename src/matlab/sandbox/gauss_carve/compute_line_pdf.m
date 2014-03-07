function p = compute_line_pdf(scan, x)
	% p = compute_line_pdf(scan, x)
	%
	%	Computes the value of the line-pdf for scan at
	%	location x.
	%
	% arguments:
	%
	%	scan - The scan statistics (see make_scandist.m)
	%	x - The location to evaluate the pdf
	%
	% output:
	%
	%	p - The value of the line-pdf at location x
	%

	% find normalized distance of x along the scan vector
	d = dot(x - scan.Es, scan.u);
	f = d/scan.dist; % fraction of x's position along line

	% we want to interpolate between the two gaussian distributions
	% at each end of this line segment. For interpolation/extrapolation,
	% we have three cases, depending if f negative, f in [0,1], or
	% f greater than 1.
	
	if(f < 0)
		% use sensor's distribution
		p = compute_bivar_gauss(scan.Es,scan.Cs,x);
	elseif(f > 1)
		% use scan point's distribution
		p = compute_bivar_gauss(scan.Ep,scan.Cp,x);
	else
		% iterpolate between two distributions
		E = scan.Es + d*scan.u; % projected position of x
		C = f*scan.Cp + (1-f)*scan.Cs; % blended covariance
		p = compute_bivar_gauss(E, C, x);
	end

	% normalize this distribution
	p = p * scan.linenorm;
end
