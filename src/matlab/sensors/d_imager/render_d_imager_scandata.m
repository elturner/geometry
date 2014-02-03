function [] = render_d_imager_scandata(scandata)
	% render_d_imager_scandata(scandata)
	%
	%	Will play the intensity images of these scandata
	%	as a movie in a new figure.
	%

	global is_paused;
	global scan_index;
	global should_exit;

	% set up figure
	figure();
	f = gcf;
	axis equal;
	set(f, 'renderer', 'opengl');
	title('Panasonic D-Imager. Time: ', 'FontSize', 15);

	% set the key handler */
	set(f,'KeyPressFcn',@KeyHandling);

	% iterate through scans
	is_paused = false;
	should_exit = false;
	scan_index = 1;
	while(~should_exit)

		% verify that scan index is valid
		if(scan_index <= 0)
			scan_index = scandata.num_scans;
		end
		if(scan_index > scandata.num_scans)
			scan_index = 1;
		end

		% get the next image frame
		M = reshape(scandata.scans(scan_index).ndat, ...
			scandata.image_width, scandata.image_height)';

		% plot the next frame of data
		figure(f);
		imagesc(M);
		title(['scan #', num2str(scan_index), ' at timestamp ', ...
			num2str(scandata.scans(scan_index).timestamp)]);
		
		% render the frame
		drawnow;

		% only move to next scan if paused
		if(is_paused)
			pause;
		else
			scan_index = scan_index + 1;
		end
	end
end

% key handler
function KeyHandling(src,evnt)
	
	global is_paused;
	global scan_index;
	global should_exit;

	switch evnt.Character
		
		% play/pause key
		case 'p'
			is_paused = ~is_paused;
		case ' '
			is_paused = ~is_paused;
		
		% move one scan earlier
		case ','
			scan_index = scan_index - 1;
		case 'a'
			scan_index = scan_index - 1;
	
		% move one scan later
		case '.'
			scan_index = scan_index + 1;
		case 'd'
			scan_index = scan_index + 1;

		% quit this
		case 'q'
			should_exit = true;
	end
end
