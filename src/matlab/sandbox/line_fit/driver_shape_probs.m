close all;
clc;

% This driver file will perform line-fitting on a set of sample URG scans
% with fss statistical metadata

% read in the sample scans
% clear all; import_sample_scans;

% the scans are now in the structure list sample_scans, iterate over them
n = length(sample_scans);
for i = 1:n
	
	% fit some lines
	[pl, cl] = compute_shape_probs(sample_scans(1), 200, 3*pi/2/1080);

	% plot the result
	figure(1);
	clf;
	hold all;
	axis equal;
	scatter(sample_scans(i).pts(1,:), sample_scans(i).pts(2,:), ...
			10, cl);
	title(['Scan timestamp: ', num2str(sample_scans(i).timestamp)],...
			'FontSize', 18);
	colorbar;
	drawnow;
end
