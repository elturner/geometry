function [V,T] = read_obj(filename)
	% [V,T] = read_obj(filename)
	%
	%	Parses the specified file as an obj.
	%	Stores the results in the matrices V and T.
	%
	% output:
	%
	%	V -	a Nx3 matrix of vertices (x,y,z)
	%	T -	a Mx3 matrix of triangles (i,j,k)
	%

	% open the file
	fid = fopen(filename, 'r');
	if(fid < 0)
		error(['Could not read ', filename]);
	end

	% initialize matrices
	V = zeros(0,3);
	T = zeros(0,3);
    num_ignored = 0;

	% parse each line of the file
	while(~feof(fid))

		% get the next line
		line = fgetl(fid);
		if(~ischar(line))
			break;
		end

		% check for blank line
		line = strtrim(line);
		if(length(line) == 0)
            num_ignored = 1+num_ignored;
			continue;
		end
		
		% parse as vertex, face, or neither
		if(line(1) == 'v' && line(2) == ' ')
	
			% attempt to parse as vertex
			[A,count] = sscanf(line, 'v %f %f %f', 3);
			if(count ~= 3)
				error(['Bad line in obj file: ', line]);
			end

			% store vertex
			V(end+1,:) = A';

		elseif(line(1) == 'f' && line(2) == ' ')

			% attempt to parse as face (with three verts)
			[A,count] = sscanf(line, 'f %d %d %d', 3);
			if(count ~= 3)
                [A,count] = sscanf(line, 'f %d/%f %d/%f %d/%f', 6);
                if(count ~= 6)
                    error(['bad line in obj file: ', line]);
                end
                A = A(1:2:end);
            end

			% store face
			T(end+1,:) = A';

		else
			% bad line, ignore
            num_ignored = 1 + num_ignored;
			continue;
		end
    end
    
    % inform user of results
    if(num_ignored > 0)
        disp(['Ignored ', num2str(num_ignored), ' line(s)']);
    end
end
