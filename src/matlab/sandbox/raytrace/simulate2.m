close all;
clear all;
clc;

% simulates one line going through many boxes

% make a random line
a = rand(3,1);
b = rand(3,1);

% plot it
figure(1);
hold all;
axis equal;
set(gcf, 'renderer', 'opengl');
render_line(a,b,'g');

% test unit-cube of boxes
for i = 0:0.1:1
	for j = 0:0.1:1
		for k = 0:0.1:1
		
			% get bounds for this box
			bounds = [(i+[0 0.1]);(j+[0 0.1]);(k+[0 0.1])];

			% test this box
			if(intersects_box(a,b,bounds))
				render_box(bounds);
			end
		end
	end
end
