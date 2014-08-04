function [R, T] = read_ptx_header(fid)
	% [R, T] = read_ptx_header(fid)
	%
	%	Given an open file handle to a .ptx formatted file,
	%	will read the header information, and store the
	%	sensor transform in the output variables.
	%
	% format:
	%
	%	The .ptx file's header should be 10 lines long:
	%
	%	<int>
	%	<int>
	%	<Tx> <Ty> <Tz>       // sensor position
	%	<Axx> <Axy> <Axz>    // oriented x-axis of sensor
	%	<Ayx> <Ayy> <Ayz>    // oriented y-axis of sensor
	%	<Azx> <Azy> <Azz>    // oriented z-axis of sensor
	%	<Axx> <Axy> <Axz> 0
	%	<Ayx> <Ayy> <Ayz> 0
	%	<Azx> <Azy> <Azz> 0
	%	<Tx> <Ty> <Tz> 1     // 4x4 transform matrix
	%
	% arguments:
	%
	%	fid -	The open file descriptor for the PTX file
	%
	% output:
	%
	%	R - 	The rotation matrix described
	%	T -	The translation vector described
	%

	% read first two lines (and ignore them)
	tline = fgetl(fid);
	tline = fgetl(fid);

	% get translation
	tline = fgetl(fid);
	[T, count] = sscanf(tline, '%f %f %f');
	if(count ~= 3)
		error('Invalid translation format: ', tline);
	end

	% get rotation
	[R, count] = fscanf(fid, '%f %f %f', [3 3]);
	if(count ~= 9)
		error('Invalid rotation format');
	end
	R = R'; % set appropriate order

	% read a 4x4 transform
	[Tk,count] = fscanf(fid, '%f %f %f %f', [4 4]);
	if(count ~= 16)
		error('Invalid 4x4 transform matrix');
	end
end
