close all;
clear all;
clc;

% simulates line segment / box intersection

% make a random box
a = rand(3,1);
b = a + rand(3,1);
bounds = [a,b];

% plot it
figure(1);
hold all;
axis equal;
set(gcf, 'renderer', 'opengl');
render_box(bounds);

% test a bunch of lines
trials = 1000;
for t = 1:trials
	
	% make a random line
	a = rand(3,1)*4 - 1;
%	b = rand(3,1)*4 - 1;
	b = a + 0.5*randn(3,1);

	% test it
	if(intersects_box(a,b,bounds))
		render_line(a,b,'r');
	else
		render_line(a,b,'g');
	end
end

