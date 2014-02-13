function fss = read_fss(filename, NUM_SCANS)
	% fss = read_fss(filename)
	%
	%	Reads the fss (Filtered with Statistics Scan) file,
	%	parses it into a structure, and returns the
	%	generated structure.
	%
	% arguments:
	%
	%	filename -	Path to fss file
	%
	%	NUM_SCANS -	Optional.  Reads only first n scans.
	%
	% output:
	%
	%	fss -	The generated structure list.  Each element in
	%		this structure list has the following fields:
	%
	%			scanner_name -	Name of generating scanner
	%			scanner_type -	Type of scanner
	%			num_scans -	Number of scans
	%			num_points_per_scan -	Num points per scan
	%			units -		The units of scan points
	%			scans -		A list of scans
	%
	%		Each element in fss.scans contains the following
	%		fields:
	%
	%			timestamp -	The synchronized timestamp
	%					of this scan (scalar)
	%			pts -		A list of [x;y;z]-coords
	%					sized (3xN)
	%			intensity -	List of intensity values,
	%					sized (1xN)
	%			bias -		List of biases, sized (1xN)
	%			stddev -	List of stddevs, sized (1xN)
	%			width -		List of widths, sized (1xN)
	%

	% open binary file for reading
	disp(['Opening ',filename,' ...']);
	disp('(this may take a while) ...');
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['Unable to open file for reading: ', filename]);
	end

	% read filetype information
	disp('reading header...');
	tline = fgetl(fid);
	if(~strcmp(tline, 'fss')) % check for magic number
		error(['Not a valid fss file: ', tline]);
	end

	% read header settings
	version = 0; % default assumption about version
	format = 'little_endian'; % default assumption about format
	fss.scanner_name = 'unknown';
	fss.scanner_type = 'unknown';
	fss.num_scans = 0;
	fss.num_points_per_scan = 0;
	fss.units = 'unknown';
	fss.scans = [];
	while(true)
		
		% get the next line in the header
		tline = fgetl(fid);

		% check for end of header
		if(strcmp(tline, 'end_header'))
			break;
		end
		
		% get the parameter type of this line
		spaces = find(tline == ' ');
		key = tline(1:spaces(1)-1);

		% swtich based on key type
		switch(key)
			case 'version'
				% update version info
				version = sscanf(tline, 'version %f');
			case 'format'
				% update format info
				format = sscanf(tline, 'format %s');
			case 'scanner_name'
				fss.scanner_name = sscanf(tline, ...
					'scanner_name %s');
			case 'scanner_type'
				fss.scanner_type = sscanf(tline, ...
					'scanner_type %s');
			case 'num_scans'
				fss.num_scans = sscanf(tline, ...
					'num_scans %d');
			case 'num_points_per_scan'
				fss.num_points_per_scan = sscanf(tline, ...
					'num_points_per_scan %d');
			case 'units'
				fss.units = sscanf(tline, 'units %s');
			otherwise
				error(['Unknown header field ', ...
				       'in fss file: ', key]);
		end
	end

	% check for optional params
	if(~exist('NUM_SCANS', 'var'))
		NUM_SCANS = fss.num_scans;
	end

	% for now, this function only supports binary reads
	if(strcmp(format, 'little_endian'))
		format = 'l';
	elseif(strcmp(format, 'big_endian'))
		format = 'b';
	else
		error(['This function only parses binary ', ...
			'formatted fss files']);
	end

	% parse body of the file
	disp('reading body...');
	N = NUM_SCANS;
	M = fss.num_points_per_scan;
	fss.scans = struct('timestamp', ...
		mat2cell(zeros(1,N), 1, ones(1,N)), ...
		'pts', mat2cell(zeros(1,N), 1, ones(1,N)), ...
		'intensity', mat2cell(zeros(1,N), 1, ones(1,N)), ...
		'bias', mat2cell(zeros(1,N), 1, ones(1,N)), ...
		'stddev', mat2cell(zeros(1,N), 1, ones(1,N)), ...
		'width', mat2cell(zeros(1,N), 1, ones(1,N)));
	for i = 1:N

		% display progress to user
		if(mod(i,10) == 0)
			disp(['reading scan #',num2str(i),...
			      '/',num2str(N),'...']);
		end

		% prepare fields for this scan
		fss.scans(i).timestamp = fread(fid, 1, 'double', 0, format);
		fss.scans(i).pts = zeros(3,M);
		fss.scans(i).intensity = zeros(1,M);
		fss.scans(i).bias = zeros(1,M);
		fss.scans(i).stddev = zeros(1,M);
		fss.scans(i).width = zeros(1,M);

		% iterate over the points in the file
		for j = 1:M
			% read in information about current point
			fss.scans(i).pts(1,j) ...
				= fread(fid, 1, 'double', 0, format);
			fss.scans(i).pts(2,j) ...
				= fread(fid, 1, 'double', 0, format);
			fss.scans(i).pts(3,j) ...
				= fread(fid, 1, 'double', 0, format);
			fss.scans(i).intensity(j) ...
				= fread(fid, 1, 'int32', 0, format);

			% read statistal information
			fss.scans(i).bias(j) = ...
				fread(fid, 1, 'double', 0, format);
			fss.scans(i).stddev(j) = ...
				fread(fid, 1, 'double', 0, format);
			fss.scans(i).width(j) = ...
				fread(fid, 1, 'double', 0, format);
		end
	end

	% clean up
	fclose(fid);
end
