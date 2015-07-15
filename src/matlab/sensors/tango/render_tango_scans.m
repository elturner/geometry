function [] = render_tango_scans(scandata)
	% RENDER_TANGO_SCANS(scandata)
	%
	%	Will allow the user to iterate through the depth
	%	frames of the tango data in a new figure.
	%

	global is_paused;
	global scan_index;
	global should_exit;

	% set up figure
	figure();
	f = gcf;
	axis equal;
	set(f, 'renderer', 'opengl');
	title('Google Tango. Time: ', 'FontSize', 15);

	% set the key handler */
	set(f,'KeyPressFcn',@KeyHandling);

	% iterate through scans
	is_paused = true;
	should_exit = false;
	scan_index = 1;
	num_scans = length(scandata);
	while(~should_exit)

		% verify that scan index is valid
		if(scan_index <= 0)
			scan_index = num_scans;
		end
		if(scan_index > num_scans)
			scan_index = 1;
		end

		% check if valid scan
		if(isempty(scandata(scan_index).depthbuf))
			% skip this one
			scan_index = scan_index + 1;
			continue;
		end

		% plot the next frame of data
		figure(f);
		scatter3(...
			scandata(scan_index).depthbuf(1,:), ...
			scandata(scan_index).depthbuf(2,:), ...
			scandata(scan_index).depthbuf(3,:), ...
			10, [ 0 0 1 ], 'filled');

		title(['scan #', num2str(scan_index), ' at timestamp ', ...
			num2str(scandata(scan_index).timestamp)]);
		
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
