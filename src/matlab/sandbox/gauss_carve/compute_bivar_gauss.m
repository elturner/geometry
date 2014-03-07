function p = compute_bivar_gauss(E,C,x)
	% p = compute_bivar_gauss(E,C,x)
	%
	%	Computes the value at x of the bivariate gaussian pdf
	%	defined by mean E and covariance C.
	%
	% arguments:
	%
	%	E,C - The mean/cov of the bivariate gaussian to compute
	%	x - The 2D location to sample the gaussian pdf
	%
	% output:
	%
	%	p - The value of the pdf at x
	%

	P = (x - E);
	p = (1/(2*pi*abs(sqrt(det(C))))) .* exp(-0.5*P'*inv(C)*P);
end
