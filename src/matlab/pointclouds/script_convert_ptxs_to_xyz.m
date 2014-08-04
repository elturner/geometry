close all;
clear all;
clc;

%%
% @file script_convert_ptxs_to_xyz.m
% @author Eric Turner <elturner@eecs.berkeley.edu>
% @brief Converts a series of .ptx files to .xyz and .mad format
%
% @section DESCRIPTION
%
% Will convert from .ptx files, which contain static point-cloud scans,
% to a format that is usable by our processing code: xyz and mad files
% for the pointcloud and pose information, respectively.
%

%----------------------------%
%-- Input Variables to Set --%
%----------------------------%

infiles = {'/home/elturner/Desktop/VmmlLab_0.ptx', ...
	'/home/elturner/Desktop/VmmlLab_1.ptx', ...
	'/home/elturner/Desktop/VmmlLab_2.ptx'};
xyzfile = '/home/elturner/Desktop/all_points.xyz';
madfile = '/home/elturner/Desktop/poses.mad';

%----------------------------%
%--  End Input Variables   --%
%----------------------------%

% prepare mad file for writing
fid = fopen(madfile, 'wb');
fwrite(fid, 0, 'int32'); % number of zupts
fwrite(fid, numel(infiles), 'int32');

% iterate through input files
for i = 1:numel(infiles)

	% get the next file to parse
	ind = i-1;
	infile = infiles{i};
	
	% print status to user
	fprintf(['Parsing file: ', infile, '\n']);

	% parse it
	tic;
	[R,T] = convert_ptx_to_xyz(ind, infile, xyzfile);
	toc;

	% record the pose info
	fwrite(fid, ind, 'double'); % timestamp
	fwrite(fid, T, 'double'); % position (units: meters)
	fwrite(fid, [0 0 0], 'double'); % bogus rotation
end

% cleanup
fclose(fid);
