function fp = rectify_fp(fp)
	% fp = rectify_fp(fp)
	%
	%	Will attempt to force all angles to be at 90 or 180
	%	degrees in the edges of this floorplan.
	%

	% parameters 
	num_iters = 1000; % how long to run
	angle_thresh = pi/180; % how close is 'close enough'?

	% optimize graph over several iterations
	for iter = 1:num_iters

		% step size based on iteration number
		step = fp.res / iter;

		% iterate over the vertices
		for i = 1:fp.num_verts

			% analyze this vertex
			[angle, bisect] = analyze_corner(fp, i);

			% check the current angle
			if(angle >= pi - angle_thresh)
				continue; % vertex rectified straight
			elseif(abs(angle - pi/2) <= angle_thresh)
				continue; % vertex already rectified right
			elseif(angle <= pi/2)
				% angle too sharp, increase it
				fp.verts(i,:) = fp.verts(i,:) + bisect*step;
			elseif(angle <= 3*pi/4)
				% angle too wide, reduce it to 90 degrees
				fp.verts(i,:) = fp.verts(i,:) - bisect*step;
			else
				% angle too sharp, increase to 180 degrees
				fp.verts(i,:) = fp.verts(i,:) + bisect*step;
			end
		end

		% display result
		if(mod(iter, 10) == 0)
			clf;
			render_fp(fp);
			drawnow;
		end
	end
end

function [angle, bisect] = analyze_corner(fp, i)
	% [angle, bisect] = analyze_corner(fp, i)
	%
	%	Will analyze the angle and bisector of the corner
	%	corresponding to the i'th vertex
	%

	% find the edges that are incident to this vertex
	ei = (fp.edges(:,1) == i | fp.edges(:,2) == i);
	if(sum(ei) ~= 2)
		% not a valid corner vertex, ignore it
		angle = 2*pi;
		bisect = [0;0];
		return;
	end

	% get edges, and orient them away from the i'th vertex
	ei = find(ei);
	e = zeros(2,2); % ordering: edge ind, vert ind
	for j = 1:2
		if(fp.edges(ei(j),1) ~= i)
			e(j,1) = fp.edges(ei(j),2);
			e(j,2) = fp.edges(ei(j),1);
		else
			e(j,1) = fp.edges(ei(j),1);
			e(j,2) = fp.edges(ei(j),2);
		end
	end
	
	% get normalized vectors along each edge
	n1 = fp.verts(e(1,2),:) - fp.verts(e(1,1),:);
	n1 = n1 / norm(n1);
	n2 = fp.verts(e(2,2),:) - fp.verts(e(2,1),:);
	n2 = n2 / norm(n2);

	% find angle and bisector
	angle = acos(dot(n1,n2));
	bisect = n1 + n2;
	bisect = bisect / norm(bisect);

	% debug
	%clf;
	%hold all;
	%axis equal;
	%render_fp(fp);
	%plot(fp.verts(i,1), fp.verts(i,2), 'ro');
	%plot(fp.verts(i,1) + [0 n1(1)], fp.verts(i,2) + [0 n1(2)], 'b-');
	%plot(fp.verts(i,1) + [0 n2(1)], fp.verts(i,2) + [0 n2(2)], 'g-');
	%plot(fp.verts(i,1) + [0 bisect(1)], fp.verts(i,2) + [0 bisect(2)],'m-');
	%disp(['angle: ', num2str(180*angle/pi)]);
	%pause;

end
