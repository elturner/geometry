function [R,T] = convert_ptx_to_xyz(ind, ptxfile, xyzfile)
	% convert_ptx_to_xyz(ind, ptxfile, xyzfile)
	%
	%	Converts a .ptx file to a .xyz file.
	%
	%	If the index is non-zero, it will append
	%	to any existing file at 'xyzfile'.  If the index
	%	given is zero, then a new file will be created.
	%
	% arguments:
	%
	%	ind -		Index of the input pointcloud
	%	ptxfiles -	The path to the input .ptx file
	%	xyzfile -	The path to the output .xyz file
	%
	% output:
	%
	%	R -	Rotation of sensor pose in ptx file
	%	T -	Translation of sensor pose in ptx file
	%

	% open the given files
	infile = fopen(ptxfile, 'r');
	if(ind == 0)
		outfile = fopen(xyzfile, 'w');
	else
		outfile = fopen(xyzfile, 'a');
	end

	% read the header to get pose info
	[R,T] = read_ptx_header(infile);

	% iterate through files contents
	linenum = 0;
	while(~feof(infile))

		% update user on status
		linenum = linenum + 1;
		if(mod(linenum, 1000000) == 0)
			fprintf('\tReading point %d:%d...\n',ind,linenum);
		end

		% get next line in file
		tline = fgetl(infile);
		if(isempty(tline))
			continue;
		end
		
		% next line has a point
		[A,count] = sscanf(tline, '%f %f %f %f %d %d %d');
		if(count ~= 7)
			error(['Unable to read line: ', tline]);
		end

		% get the point and convert to world coordinates
		P = R*A(1:3) + T;

		% convert info
		x = P(1) * 1000; % convert from meters to mm
		y = P(2) * 1000;
		z = P(3) * 1000;
		intensity = A(4); % get metadata about point
		red = A(5);
		green = A(6);
		blue = A(7);
		index = ind;
		timestamp = ind;
		serial = 0;

		% write output
		fprintf(outfile, '%f %f %f %d %d %d %d %f %d\n', ...
			x, y, z, red, green, blue, ...
			index, timestamp, serial);
	end

	% clean up
	fclose(infile);
	fclose(outfile);
end
