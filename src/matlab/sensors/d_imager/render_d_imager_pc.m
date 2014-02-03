function [] = render_d_imager_pc(scan)
	% render_d_imager_pc(scan)
	%
	%	Renders the pointcloud for a single d-imager scan
	%

	% initialize figure
	clf;
	axis equal;
	set(gcf, 'renderer', 'opengl');

	% plot points to figure
	scatter3(scan.xdat, scan.ydat, scan.zdat, 10, scan.ndat, 'filled');

end
