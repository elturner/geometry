function area = compute_area(fp)
	% area = compute_area(fp)
	%
	%	Given a floorplan structure as generated from read_fp.m,
	%	will find the area covered by the interior space defined
	%	by the floorplan.
	%
	% arguments:
	%
	%	fp -	The floor plan structure
	%
	% output:
	%
	%	area -	The area of the floorplan (units: meters^2)
	%

	% initialize sum
	area = 0;

	% iterate over triangles
	for i = 1:fp.num_tris
		
		% get points of current triangle
		P = fp.verts(fp.tris(i,:), :);

		% get area of this triangle
		a = tri_area(P(1,:), P(2,:), P(3,:));
		area = area + a;
	end
end

function A = tri_area(P1, P2, P3)
% A = tri_area(P1, P2, P3)
%
% DESC:
% calculates the triangle area given the triangle vertices (using Heron's
% formula)
%
% AUTHOR
% Marco Zuliani - zuliani@ece.ucsb.edu
%
% VERSION:
% 1.0
%
% INPUT:
% P1, P2, P3 = triangle vertices
%
% OUTPUT:
% A          = triangle area

u1 = P1 - P2;
u2 = P1 - P3;
u3 = P3 - P2;

a = norm(u1);
b = norm(u2);
c = norm(u3);

% Stabilized Heron formula
% see: http://http.cs.berkeley.edu/%7Ewkahan/Triangle.pdf
%
% s = semiperimeter
% A = sqrt(s * (s-a) * (s-b) * (s-c))

% sort the elements
v = sort([a b c]);
a = v(3);
b = v(2);
c = v(1);

temp = b + c;
v1 = a + temp;
temp = a - b;
v2 = c - temp;
v3 = c + temp;
temp = b - c;
v4 = a + temp;
A = 0.25 * sqrt(v1*v2*v3*v4);

end
