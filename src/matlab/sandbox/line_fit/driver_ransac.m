close all;
clc;

% This driver file will perform line-fitting on a set of sample URG scans
% with fss statistical metadata

% read in the sample scans
clear all; import_sample_scans;

% the scans are now in the structure list sample_scans, iterate over them
n = length(sample_scans);
for i = 1:n
	
	% fit some lines
	[line_models, P_line, P_corner] = line_fit(sample_scans(i));

	% plot the result
	render_fss(sample_scans(i), line_models);
	pause(0.1);
end
