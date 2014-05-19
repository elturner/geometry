function [] = render_noisypath(path)
	% render_noisypath(path)
	%
	%	Renders the path that has been imported from a call
	%	to read_noisypath(...)
	%
	% arguments:
	%
	%	path - 	The path structure to render
	%

	% import libraries
	addpath('../carve/');

	% prepare figure
	hold all;
	axis equal;

	% iterate over path
	for i = 1:length(path.poses)
		% render the current pose as a gaussian
		render_gauss3d(path.poses(i).pos_mean, ...
		               path.poses(i).pos_cov);
	end
end
