function [] = render_line(a, b, c)
	% render_line(a, b, c)
	%
	%	Renders line segment (a,b) with color c
	%
	% arguments:
	%
	%	a,b -	3d vectors representing endpoints
	%	c -	String representing color (e.g. 'k', 'b', 'g')
	%

	plot3([a(1) b(1)], [a(2) b(2)], [a(3) b(3)], ['-', c]);
end
