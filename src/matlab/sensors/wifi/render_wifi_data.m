%%
% @file render_wifi_data.m
% @author Eric Turner <elturner@eecs.berkeley.edu>
%
% @section DESCRIPTION
%
% Matlab function that will plot the packets on a
% human readable freq vs time plot.
%

function [] = render_wifi_data(data)
	% render_wifi_data(data)
	%
	%	Will render the wifi data on a new plot
	%

	% prepare figure
	hold all;
	title('WiFi Data Capture', 'FontSize', 18);
	xlabel('Windows Time (milliseconds)', 'FontSize', 14);
	ylabel('Frequency (MHz)', 'FontSize', 14);

	% iterate over packets
	x = zeros(data.num_scans, 1);
	y = zeros(data.num_scans, 1);
	for i = [1:data.num_scans]

		% record current scan as a point
		x(i) = data.scans(i).win_time;
		y(i) = data.scans(i).freq;
	end
	
	% plot it
	plot(x, y, '.');
end
