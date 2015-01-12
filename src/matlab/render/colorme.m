function colors = colorme(array)
	% colors = colorme(array)
	%
	%	Given a set of N values as an array of scalars, 
	%	this function will return a Nx3 matrix representing
	%	a color for each given value.
	%
	% arguments:
	%
	%	array -		Either a row or column vector of scalars
	%
	% output:
	%
	%	colors -	The colors corresponding with each input
	%			value
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	Written January 12, 2015
	%

	% get bounds
	N = numel(array);
	minval = min(array);
	maxval = max(array);
	res = 255;

	% make a JETMAP array
	J = jet(res);

	% get indices for each input value
	I = round( res .* (array - minval) ./ (maxval - minval) )

	% create colors
	colors = J(I,:);
end
