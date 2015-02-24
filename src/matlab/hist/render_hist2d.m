function [] = render_hist2d(H, draw_3d, threshold)
	% RENDER_HIST2D(H)
	% RENDER_HIST2D(H, draw_3d)
	% RENDER_HIST2D(H, draw_3d, threshold)
	%
	%	Given a 2D histogram, will render it as a scaled image
	%
	% arguments:
	%
	%	H -	An Nx3 matrix, where each row is (x,y,weight)
	%		Where x and y are indices (integers).
	%
	%	draw_3d -	If specified and true, then will
	%			draw as a 3D triangulation.
	%
	%	threshold -	If specified, will render as binary image,
	%			based on if weights are above or below
	%			the given threshold.
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on February 23, 2015
	%

	% look at arguments
	if(~exist('draw_3d', 'var') || isempty(draw_3d))
		draw_3d = false;
	end
	if(~exist('threshold', 'var') || isempty(threshold))
		threshold = [];
	end

	% analyze size of the histogram area
	offx = min(H(:,1));
	offy = min(H(:,2));
	lenx = max(H(:,1)) - offx;
	leny = max(H(:,2)) - offy;

	% make a dense matrix
	M = zeros(leny, lenx);
	if(isempty(threshold))
		% draw each sample as a pixel
		for i = 1:size(H,1)
			M(H(i,2) - offy + 1, H(i,1) - offx + 1) = H(i,3);
		end
	else
		% threshold each sample for discretized pixels
		for i = 1:size(H,1)
			M(H(i,2) - offy + 1, H(i,1) - offx + 1) ...
					= searchlist(threshold, H(i,3))-1;
		end
	end

	% render the image
	if(draw_3d)
		% draw a 3D plot
		surf(M, 'EdgeAlpha', 0);
	else
		% flip and draw in image coordinates
		M = flipud(M);
		imagesc(M);
	end

	% regardless, we want axis equal
	axis equal;
end

function idx = searchlist(list, val)
	% idx = searchlist(list, val)
	%
	%	Given sorted list, will return index of val
	%	if it were in the list
	%

	for i = 1:length(list)
		if(val < list(i))
			idx = i;
			return;
		end
	end

	% it's at the end of the list
	idx = length(list)+1;
	return;
end
