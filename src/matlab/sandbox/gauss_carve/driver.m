close all;
clear all;
clc;

% this driver shows an example carve map calculation
% it goes from a sensor/scanpoint point pdf, and 
% to compute the carve map from that

% make a test scan
scan = make_scandist([-7;2],[0.4 -0.35; -0.35 0.4],[3;4],[1 -0.6; -0.6 1]);
%scan = make_scandist([-7;-8],[1 -0.7; -0.7 1],[15;20],[1 0.9; 0.9 1]);

% plot the various distributions
render_point_pdfs(scan);
title('Point Distributions');

%render_line_pdf(scan);
%title('Line Distribution');

render_carve_map(scan);
title('Carve Map');
