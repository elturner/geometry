%%
% @file read_wifi_data.m
% @author Eric Turner <elturner@eecs.berkeley.edu>
%
% @section DESCRIPTION
%
% Matlab function that will read the binary data from 
% wifi output files.
%

function data = read_wifi_data(filename)
	% data = read_wifi_data(filename)
	%
	%	Parses the specified file, and populates
	%	the output structure with the wifi packets
	%	that are represented in this file.
	%
	% arguments:
	%
	%	filename -	The binary file to parse
	%
	% output:
	%
	%	data -		The output struct, which
	%			contains the following fields:
	%
	%			ver_maj -	Major version number
	%			ver_min -	Minor version number
	%			name -		Name of antenna
	%			num_scans -	Number of scans in file
	%			scans -		Array of structs for each
	%					scan
	%
	%			Each scan in the scans array is a struct
	%			with the following fields:
	%
	%			time_sec -	Seconds component of the
	%					hardware time
	%			time_usec -	Microseconds component of
	%					the hardware time
	%			win_time -	The windows timestamp,
	%                                       in units of cycles
	%			bssid -		Array of three numbers
	%					representing bssid values
	%			sig_level -	Radio signal level
	%			freq -		Radio frequency value
	%			tag_num -	Frame tag number
	%			tag_len -	length of SSID tag
	%			ssid -		The ssid tag of frame
	%

	% attempt to open file
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['could not open file: ', filename]);
	end

	% check that file is a wifi data file
	magic = fread(fid, 5, 'schar');
	if(any(magic ~= [87 73 70 73 0]'))
		% wrong file type
		fclose(fid);
		error(['not a wifi data file: ', filename]);
	end

	% parse version info
	data.ver_maj = fread(fid, 1, 'uint8');
	data.ver_min = fread(fid, 1, 'uint8');
	
	% parse sensor name
	data.name = '';
	while(true)
		c = fread(fid, 1, 'schar');
		if(c == 0)
			break;
		end
		data.name = [data.name, c];
	end

	% parse number of scans
	data.num_scans = fread(fid, 1, 'uint32');
	
	% prepare scans array
	arr = cell(data.num_scans, 1);
	data.scans = struct('time_sec', arr, 'time_usec', arr, ...
	                    'win_time', arr, 'bssid', arr, ...
			    'sig_level', arr, 'freq', arr, ...
			    'tag_num', arr, 'tag_len', arr, ...
			    'ssid', arr);

	% iterate through scans
	for i = [1:data.num_scans]

		% check that we haven't reached end of file
		if(feof(fid))
			fclose(fid)
			error('unexpected end-of-file at scan #', ...
			      num2str(i));
		end

		% read the next scan
		data.scans(i).time_sec  = fread(fid, 1, 'uint32');
		data.scans(i).time_usec = fread(fid, 1, 'uint32');
		data.scans(i).win_time  = fread(fid, 1, 'uint64');
		data.scans(i).bssid     = fread(fid, 3, 'int16');
		data.scans(i).sig_level = fread(fid, 1, 'int8');
		data.scans(i).freq      = fread(fid, 1, 'uint16');
		data.scans(i).tag_num   = fread(fid, 1, 'uint8');
		data.scans(i).tag_len   = fread(fid, 1, 'uint8');
		data.scans(i).ssid      = fread(fid, ...
		                          data.scans(i).tag_len, ...
					  'uint8=>char')';
	end

	% clean up
	fclose(fid);
end
