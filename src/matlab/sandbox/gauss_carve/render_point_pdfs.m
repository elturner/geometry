function [] = render_point_pdfs(scan)
	% render_point_pdfs(scan)
	%
	%	Renders the gaussian distribution of point
	%	and sensor positions, given a 2D environment.
	%
	% arguments:
	%
	%	scan - The scan statistics (see make_scandist.m)
	%

	% prepare figure properties
	figure;
	hold all;
	set(gcf, 'renderer', 'opengl');

	% sample the sensor distribution and plot
	[X,Y,Z] = bivar_gauss(scan.Es,scan.Cs);
	surf(X,Y,Z);

	% sample and plot the point distribution
	[X,Y,Z] = bivar_gauss(scan.Ep,scan.Cp);
	surf(X,Y,Z);
end

function [X,Y,Z] = bivar_gauss(E,C)
	% [X,Y,Z] = bivar_gauss(E,C)
	%
	%	Generates a sampling of a bivariate gaussian
	%	distribution with the given mean (E) and covariance
	%	matrix (C).
	%

	% sample in a grid around the mean
	[X,Y] = meshgrid(E(1)+3*C(1,1)*[-1:0.1:1], ...
	                 E(2)+3*C(2,2)*[-1:0.1:1]);
	P = [X(:)' - E(1); Y(:)' - E(2)];

	% bivariate gaussian pdf equation
	Z = zeros(size(X));
	for i = 1:numel(X)
		Z(i) = (1/(2*pi*abs(sqrt(det(C))))) ...
				.* exp(-0.5*P(:,i)'*inv(C)*P(:,i));
	end
end
