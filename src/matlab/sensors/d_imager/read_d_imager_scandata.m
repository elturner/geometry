function [scandata, ts] = read_d_imager_scandata(filename, n)
	% [scandata, ts] = read_d_imager_scandata(filename, n)
	%
	%	Will parse the data output of the Panasonic
	%	D-Imager and store the data in the output
	%	struct.
	%
	% arguments:
	%
	%	filename -	The file to parse
	%	n        -	OPTIONAL. Reads in the first n frames.
	%
	% output:
	%
	%	scandata -	The struct that contains
	%			the data information.
	%
	%	ts       -	A list of retrieved timestamps
	%
	% struct fields for scandata:
	%
	%	image_width  -	The width of each scan (pixels)	
	%	image_height -	The height of each scan (pixels)
	%	fps          -	The scan rate used (frames per second)
	%	freq         -	The scan frequency used (either 0, 1, 2)
	%	num_scans    -	The number of scans taken
	%	scans        -  The list of scans, each scan is a struct
	%
	% struct fields for scan:
	%
	%	timestamp    -	The timestamp of this scan (windows time)
	%	xdat         -	The x-coords of the scan points (mm)
	%	ydat         -	The y-coords of the scan points (mm)
	%	zdat         -	The z-coords of the scan points (mm)
	%	ndat         -	The grayscale intensity [0-255]
	%

	% check arguments
	if(~exist('n', 'var'))
		n = Inf;
	end

	% open the file for reading in binary
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['Unable to read file: ', filename]);
	end

	% check for magic number in the file, should be 'dimager'
	M = fread(fid, 8, 'schar');
	M_lookup = [100, 105, 109, 97, 103, 101, 114, 0]';
	if(any(M ~= M_lookup))
		% wrong file type (doesn't have magic number)
		fclose(fid);
		error('Unrecognized file type');
	end
	
	% read header information
	scandata.image_width  = fread(fid, 1, 'uint32');
	scandata.image_height = fread(fid, 1, 'uint32');
	scandata.fps          = fread(fid, 1, 'uint32');
	scandata.freq         = fread(fid, 1, 'uint32');
	scandata.num_scans    = fread(fid, 1, 'uint32');
	scandata.scans        = struct('timestamp', -1, 'xdat', [], ...
				'ydat', [], 'zdat', [], 'ndat', []);
	ts = [];

	% read each scan
	pts_per_scan = scandata.image_width * scandata.image_height;
	scan_idx = 0;
	while(~feof(fid) && scandata.num_scans < n)
	
		% read in timestamp
		t = fread(fid, 1, 'uint64');
		if(isempty(t))
			disp(['Could not read timestamp ', ...
			      (scandata.num_scans+1)]);
			continue
		end
		
		% increment scan counter
		scan_idx = scan_idx + 1;

		% store timestamp
		scandata.scans(scan_idx).timestamp = t;
		ts(scan_idx) = t;

		% read in pointcloud
		scandata.scans(scan_idx).xdat = ...
					fread(fid, pts_per_scan, 'int16');
		scandata.scans(scan_idx).ydat = ...
					fread(fid, pts_per_scan, 'int16');
		scandata.scans(scan_idx).zdat = ...
					fread(fid, pts_per_scan, 'int16');
		scandata.scans(scan_idx).ndat = ...
					fread(fid, pts_per_scan, 'uint16');
	end

	% clean up
	fclose(fid);
end
