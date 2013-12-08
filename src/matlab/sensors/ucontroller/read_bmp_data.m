%
% read_bmp_data.m
% Nicholas Corso
%
% [data,header] = read_bmp_data(filename)
%
% This function reads the data stored in a 1.0v bmp binary file.  
%
% Inputs:
%   filename - the name of the file you want to read.
%
% Outputs:
%   data - The data stored in the binary file.
%   header - The header information stored in the file.
function [data,header] = read_bmp_data(filename)


    % First thing we do is read the laser header... this is also
    % a validity check on the file
    [header, skip_to_pos] = read_bmp_header(filename);
    
    % Next we need to open the file and skip ahead to the correct location
    % in the file
    fid = fopen(filename(filename~='"'));
    fseek(fid,skip_to_pos,'bof');
    
    % Initialize data depending on how many sensors are in the 
    % file
    data.timestamps = zeros(2,header.num_readings);
    data.temperature = zeros(1,header.num_readings); 
    data.pressure = zeros(1,header.num_readings);

    % Read the actual data out
    for i = 1:1:header.num_readings
       data.timestamps(1,i) = fread(fid,1,'uint32');
       data.temperature(i) = fread(fid,1,'uint16','b');
       data.timestamps(2,i) = fread(fid,1,'uint32');
       
       temp_pressure = fread(fid,1,'uint16','b');
       temp_xlsb = fread(fid,1,'uint8');
       data.pressure(i) = bitshift((bitshift(temp_pressure,8)+temp_xlsb),-(8-header.oversampling));
       
    end
        
    % Clean up
    fclose(fid);


end