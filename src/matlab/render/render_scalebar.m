function [] = render_scalebar(side, s)
	% render_scalebar:
	%
	%	Draws a scalebar on the current figure.
	%
	% optional:
	%
	%	render_scalebar(side) will render the scalebar
	%	on the bottom left (default) if side=false, and
	%	on the bottom right if side=true
	%

	% prepare figure
	hold all;
	axis manual;

	% get position
	pos = 0.1;
	if(exist('side', 'var'))
		if(side)
			pos = 0.6;
		end
	end
	
	% determine the dimensions to make the scalebar
	xs = xlim;
	ys = ylim;

	xo = xs(1) + (xs(2) - xs(1))*pos;
	xl = floor( (xs(2) - xs(1))*0.3 );
	if(exist('s', 'var'))
		xl = s;
	end

	yo = ys(1) + (ys(2) - ys(1))*0.05;
	yl = (ys(2) - ys(1))*0.025;

	% draw it
	Xq = [ 0 0  xl xl ]/4;
	Yq = [ 0 yl yl 0  ]/4;
	Xh = [ 0 0  xl xl ]/2;
	Yh = [ 0 yl yl 0  ]/2;
	X =  [ 0 0  xl xl ];
	Y =  [ 0 yl yl 0  ];
	
	
	fill(xo + X,  yo + Y, 'k', 'LineWidth', 2);
	fill(xo + Xh, yo + Yh, 'w', 'LineWidth', 2);
	fill(xo + Xq, yo + yl/2 + Yh, 'w', 'LineWidth', 2);
	fill(xo + xl/2 + Xq, yo + yl/2 + Yh, 'w', 'LineWidth', 2);
	text(xo, yo - yl/2, num2str(0,2));
	text(xo + xl/2, yo - yl/2, num2str(xl/2,2));
	text(xo + xl, yo - yl/2, num2str(xl,2));
end
