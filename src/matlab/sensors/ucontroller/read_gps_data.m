%
% read_gps_data.m
% Nicholas Corso
%
% [data,header] = read_gps_data(filename)
%
% This function reads the data stored in a 1.0v gps binary file.  
%
% Inputs:
%   filename - the name of the file you want to read.
%
% Outputs:
%   data - The data stored in the binary file.  This is timestamps, range
%          data and optionally intensity data if that was stored in the 
%          file.
%   header - The header information stored in the file.
function [data,header] = read_gps_data(filename)


    % First thing we do is read the laser header... this is also
    % a validity check on the file
    [header, skip_to_pos] = read_gps_header(filename);
    
    % Next we need to open the file and skip ahead to the correct location
    % in the file
    fid = fopen(filename(filename~='"'));
    fseek(fid,skip_to_pos,'bof');
    
    % Initialize the data
    data.readings = cell(1,header.num_scans);
    data.timestamps = zeros(1,header.num_scans);
    
    % Read out the data
    for i = 1:1:header.num_scans
        data.timestamps(i) = fread(fid,1,'uint64');
        num_chars = fread(fid,1,'uint32');
        data.readings{i} = fread(fid,num_chars,'*char')';
    end
    
    % Clean up
    fclose(fid);


end
