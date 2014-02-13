function [] = render_fss(fss)
	% render_fss(fss)
	%
	%	Will play the fss scans in the specified struct
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
	title(fss.scanner_name, 'FontSize', 15);

	% set the key handler */
	set(f,'KeyPressFcn',@KeyHandling);

	% iterate through scans
	is_paused = false;
	should_exit = false;
	scan_index = 1;
	N = length(fss.scans);
	while(~should_exit)

		% verify that scan index is valid
		if(scan_index <= 0)
			scan_index = N;
		end
		if(scan_index > N)
			scan_index = 1;
		end

		% plot the next frame of data
		figure(f);
		[az,el] = view();
		clf;
		scatter3(fss.scans(scan_index).pts(1,:), ...
			fss.scans(scan_index).pts(2,:), ...
			fss.scans(scan_index).pts(3,:), 10, ...
			fss.scans(scan_index).stddev);
		title([fss.scanner_name, ', scan #', ...
			num2str(scan_index), ' at timestamp ', ...
			num2str(fss.scans(scan_index).timestamp)]);
		view(az,el);

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
