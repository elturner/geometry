%
% read_hmc_data.m
% Nicholas Corso
%
% [data,header] = read_hmc_data(filename)
%
% This function reads the data stored in a 1.0v hmc binary file.  
%
% Inputs:
%   filename - the name of the file you want to read.
%
% Outputs:
%   data - The data stored in the binary file.
%   header - The header information stored in the file.
function [data,header] = read_hmc_data(filename)


    % First thing we do is read the laser header... this is also
    % a validity check on the file
    [header, skip_to_pos] = read_hmc_header(filename);
    
    % Next we need to open the file and skip ahead to the correct location
    % in the file
    fid = fopen(filename(filename~='"'));
    fseek(fid,skip_to_pos,'bof');
    
    % Initialize data depending on how many sensors are in the 
    % file
    for i = 1:1:header.num_sensors
        data.timestamps = zeros(1,header.num_readings);
        data.sensors(i).readings = zeros(3,header.num_readings); 
    end
    
    % Read the actual data out
    for i = 1:1:header.num_readings
       data.timestamps(i) = fread(fid,1,'uint32');
       for j = 1:1:header.num_sensors
           data.sensors(j).readings(:,i) = fread(fid,3,'int16','b');
       end
    end
        
    % Clean up
    fclose(fid);


end