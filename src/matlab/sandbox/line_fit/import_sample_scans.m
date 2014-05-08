%%
% @file import_sample_scans.m
% @author Eric Turner <elturner@eecs.berkeley.edu>
% @brief Will read a designated test dataset for sample fss files
%
% @section DESCRIPTION
%
% Will retrieve a set of sample fss scan frames from a hard-coded
% test dataset.  The result will be stored in 'sample_scans'.
%

% import necessary libraries
addpath('../../fss');

% read sample frames from the given test file
fssfile = '../../../../../../data/20140421-2/data/urg/H1311822/urg_H1311822_scandata.fss';

% parse the file
fss = read_fss(fssfile, 300); % only read the first hundred scans
sample_scans = fss.scans;

% the sample scans are now stored in the sample_scans structure list
clear fssfile;
clear fss;
