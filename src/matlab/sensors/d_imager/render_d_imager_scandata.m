function [] = render_d_imager_scandata(scandata)
	% render_d_imager_scandata(scandata)
	%
	%	Will play the intensity images of these scandata
	%	as a movie in a new figure.
	%

	% set up figure
	figure();
	f = gcf;
	axis equal;
	set(f, 'renderer', 'opengl');
	title('Panasonic D-Imager. Time: ', 'FontSize', 15);

	% iterate through scans
	for i = 1:scandata.num_scans

		% get the next image frame
		M = reshape(scandata.scans(i).ndat, ...
			scandata.image_width, scandata.image_height)';

		% plot the next frame of data
		figure(f);
		imagesc(M);
		disp(scandata.scans(i).timestamp);
		
		% render the frame
		drawnow;
	end
end
