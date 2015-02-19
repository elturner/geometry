function [mapData] = read_wall_samples(mapFileName)
	% [mapData] = READ_WALL_SAMPLES(filename)
	%
	%	Read the specified *.dq file, and
	%	store in the struct mapData
	%

	fid = fopen(mapFileName);
	if(fid == -1) 
		error(['unable to read map file: ', mapFileName]);
	end

	% Read the header information
	tline = fgets(fid);
	mapData.maxDepth = str2num(tline);
	tline = fgets(fid);
	mapData.halfwidth = str2num(tline);
	tline = fgets(fid);
	mapData.rootNodePosition = str2num(tline)';
	tline = fgets(fid);

	% initialize the fields
	pts = textscan(fid, '%f %f %*[^\n]');
	mapData.pos = [ pts{1}' ; 
			pts{2}' ];
				% read first two numbers, which are points
	mapData.heightRange = []; % we don't care about this
	mapData.numPoints   = []; % we don't care
	mapData.poseIdx     = []; % we don't care

	% clean up
	fclose(fid);
end
