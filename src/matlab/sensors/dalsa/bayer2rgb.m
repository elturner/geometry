function J = bayer2rgb(I)
	% J = bayer2rgb(I)
	%
	%	Converts from bayer gbrg to rgb.
	%	Super-simple method.  Produces an image
	%	that is only half the size in each dimension.
	%
	
	rows = size(I, 1);
	cols = size(I, 2);

	red_map = false(rows, cols);
	red_map(2:2:rows, 1:2:cols) = true;
	
	green_map = false(rows, cols);
	green_map(1:2:rows, 1:2:cols) = true;
	%green_map(2:2:rows, 2:2:cols) = true;
	
	blue_map = false(rows, cols);
	blue_map(1:2:rows, 2:2:cols) = true;
	
	J = zeros(rows/2, cols/2, 3);
	J(:,:,1) = reshape(I(red_map), rows/2, cols/2);
	J(:,:,2) = reshape(I(green_map), rows/2, cols/2);
	J(:,:,3) = reshape(I(blue_map), rows/2, cols/2);
    J = uint8(J);


end
