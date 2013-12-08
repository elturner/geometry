close all;
clear all;
clc;

% This script will load laser timestamp information, and estimate
% the percentage of dropped scans during the corresponding data
% collection.

laser1 = load('~/Desktop/urg_H1214149_timestamps.txt');
laser2 = load('~/Desktop/urg_H1214157_timestamps.txt');

% compute percentage of received scans for each
p1 = size(laser1, 1) / ((laser1(end,1) - laser1(1,1)) * 40 / 1000);
p2 = size(laser2, 1) / ((laser2(end,1) - laser2(1,1)) * 40 / 1000);

% plot each laser
figure(1);
plot(laser1(1:end-1,1)/1000, diff(laser1(:,1))/1000, 'b-o');
title(['H1214149 -- keep rate: ', num2str(p1)], 'FontSize', 15);
xlabel('Time (sec)');
ylabel('\Delta Time (sec)');

% plot each laser
figure(2);
plot(laser2(1:end-1,1)/1000, diff(laser2(:,1))/1000, 'b-o');
title(['H1214157 -- keep rate: ', num2str(p2)], 'FontSize', 15);
xlabel('Time (sec)');
ylabel('\Delta Time (sec)');
