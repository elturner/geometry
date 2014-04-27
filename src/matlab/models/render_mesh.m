function [] = render_mesh(verts, tris, tri_colors)
	% render_mesh(verts, tris)
	%
	%	Displays the mesh.
	%
	% optional arguments:
	%
	%	render_mesh(verts, tris, tri_colors):
	%
	%		tri_colors is a length-M array (where M
	%		is the number of triangles).  Each element
	%		is a char:  'r', 'g', 'b', 'y', 'c', 'm', 'w', 'k'

	xs = verts(:,1);
	ys = verts(:,2);

    if(size(verts,2) >= 3)
        zs = verts(:,3);
    end
    
	% check for optional args
	N = size(verts, 1);
	M = size(tris, 1);
	if(~exist('tri_colors', 'var'))
		tri_colors = char('w' * ones(M,1));
	end

	% render triangles
	axis equal;
	hold all;
	for ti = 1:size(tris,1)

		t = tris(ti,:);

		% draw triangle
        if(size(verts,2) >= 3)
            patch(xs(t), ys(t), zs(t), tri_colors(ti), 'FaceAlpha', 0.5);
        else
            patch(xs(t), ys(t), tri_colors(ti));
        end
	end
	set(gcf, 'renderer', 'opengl');
end
