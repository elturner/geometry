%
% read_urg_data.m
% Nicholas Corso
%
% [data,header] = read_urg_data(filename)
%
% This function reads the data stored in a 1.0v urg binary file.  
%
% Inputs:
%   filename - the name of the file you want to read.
%
% Outputs:
%   data - The data stored in the binary file.  This is timestamps, range
%          data and optionally intensity data if that was stored in the 
%          file.
%   header - The header information stored in the file.
function [data,header] = read_urg_data(filename)


    % First thing we do is read the laser header... this is also
    % a validity check on the file
    [header, skip_to_pos] = read_urg_header(filename);
    
    % Next we need to open the file and skip ahead to the correct location
    % in the file
    fid = fopen(filename(filename~='"'));
    fseek(fid,skip_to_pos,'bof');
    
    % Initialize data depending on if intensity information is burried
    % in the file
    if(header.capture_mode == 0)
        data.range = cell(1,header.num_scans);
        data.timestamps = zeros(1,header.num_scans);
    elseif(header.capture_mode == 1)
        data.range = cell(1,header.num_scans);
        data.intensity = cell(1,header.num_scans);
        data.timestamps = zeros(1,header.num_scans);
    end
    
    % Read the data out of the file
    for i = 1:1:header.num_scans
       data.timestamps(i) = fread(fid,1,'uint32'); 
       data.range{i} = fread(fid,header.points_per_scan,'uint32');
       if( header.capture_mode == 1)
          data.intensity{i} = fread(fid,header.points_per_scan,'uint32');
       end
    end
    
    % Clean up
    fclose(fid);


end