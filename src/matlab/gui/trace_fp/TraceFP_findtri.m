function ind = TraceFP_findtri(handles)
	% ind = TraceFP_findtri(handles)
	%
	%	Will prompt user to select a triangle.
	%	Returns the index of selected triangle, or
	%	-1 if no triangle selected.
	%
	% written by Eric Turner <elturner@eecs.berkeley.edu>
	% created on February 9, 2015
	%

	% initially set to be invalid
	ind = -1;
	
	% check if any triangles exist
	N = size(handles.triangles,1);
	if(N == 0)
		fprintf('[TraceFp]\t\tNo triangles defined.\n');
		return;
	end

	% get a point from user
	[X,Y] = ginput(1);

	% iterate through triangles (I know this isn't very efficient,
	% but patience is a virtue for the user, not the programmer)
	%
	% ... if it bugs you so much, then you fix it.
	for i = 1:N

		% check if point inside triangle
		VX = handles.control_points(handles.triangles(i,:), 1);
		VY = handles.control_points(handles.triangles(i,:), 2);
		if(inpolygon(X, Y, VX, VY))
			% found it!
			ind = i;
			return;
		end
	end
end
