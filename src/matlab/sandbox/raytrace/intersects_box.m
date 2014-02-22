function x = intersects_box(a, b, bounds)
	% x = intersects_box(a, b, bounds)
	%
	%	Returns true iff the line segment defined by 3D points (a,b)
	%	intersects the axis-aligned box defined by bounds.
	%
	% arguments:
	%
	%	a,b -      The 3D points defining a line segment
	%	bounds -   A 3x2 matrix defining bounds of a box.  First
	%	           column defines min corner, second col defines
	%	           the max corner.
	%
	% output:
	%
	%	x - 	Is true iff (a,b) intersects the box
	%

	x = false;
	
	% define ray properties
	orig = a;
	invdir = 1 ./ (b-a);
	s = (invdir < 0);

	% test box intersection

	% x-coords
	tmin = (bounds(1,1+s(1)) - orig(1)) * invdir(1);
	tmax = (bounds(1,2-s(1)) - orig(1)) * invdir(1);

	% y-coords
	tymin = (bounds(2,1+s(2)) - orig(2)) * invdir(2);
	tymax = (bounds(2,2-s(2)) - orig(2)) * invdir(2);

	% test 2D intersection
	if( (tmin > tymax) || (tymin > tmax) )
		return;
	end
	
	% restrict ray bounds
	if(tymin > tmin)
		tmin = tymin;
	end
	if(tymax < tmax)
		tmax = tymax;
	end

	% z-coords
	tzmin = (bounds(3,1+s(3)) - orig(3)) * invdir(3);
	tzmax = (bounds(3,2-s(3)) - orig(3)) * invdir(3);

	% check 3D intersection
	if((tmin > tzmax) || (tzmin > tmax))
		return;
	end

	% restrict
	if(tzmin > tmin)
		tmin = tzmin;
	end
	if(tzmax < tmax)
		tmax = tzmax;
	end

	% check if line is too short to intersect
	if(tmin > tmax || tmin > 1 || tmax < 0)
		return;
	end

	% intersection!
	x = true;
	return;
end
