function [] = render_gauss3d(M, C)
	% render_gauss3d(M, C)
	%
	%	Will render a representation of a 3D gaussian in the
	%	current figure by drawing a mesh for the 90% isosurface
	%	of the gaussian.
	%
	% arguments:
	%
	%	M - 	The 3x1 mean of the distribution
	%	C -	The 3x3 covariance matrix
	%

	% the square-root of the covariance matrix represents the
	% tranformation of white noise to this distribution
	R = 2*real(C^0.5); % two standard deviations

	% vertices of mesh
	cxp = M + R(:,1);
	cxn = M - R(:,1);
	cyp = M + R(:,2);
	cyn = M - R(:,2);
	czp = M + R(:,3);
	czn = M - R(:,3);

	% faces of mesh
	X = [	cxp(1),cyp(1),cxn(1),cyn(1),cyp(1),cxn(1),cyn(1),cxp(1);
		cyp(1),cxn(1),cyn(1),cxp(1),cxp(1),cyp(1),cxn(1),cyn(1);
		czp(1),czp(1),czp(1),czp(1),czn(1),czn(1),czn(1),czn(1)];
	Y = [	cxp(2),cyp(2),cxn(2),cyn(2),cyp(2),cxn(2),cyn(2),cxp(2);
		cyp(2),cxn(2),cyn(2),cxp(2),cxp(2),cyp(2),cxn(2),cyn(2);
		czp(2),czp(2),czp(2),czp(2),czn(2),czn(2),czn(2),czn(2)];
	Z = [	cxp(3),cyp(3),cxn(3),cyn(3),cyp(3),cxn(3),cyn(3),cxp(3);
		cyp(3),cxn(3),cyn(3),cxp(3),cxp(3),cyp(3),cxn(3),cyn(3);
		czp(3),czp(3),czp(3),czp(3),czn(3),czn(3),czn(3),czn(3)];

	% plot it
	hold all;
	axis equal;
	set(gcf, 'renderer', 'opengl');
	plot3(M(1), M(2), M(3), 'ob');
	fill3(X,Y,Z,'b');
end
