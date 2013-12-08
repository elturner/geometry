%
% read_ic4_data.m
% Nicholas Corso
%
% [data,header] = read_ic4_data(filename)
%
% This function reads the data stored in a 1.0v ic4 binary file.  
%
% Inputs:
%   filename - the name of the file you want to read.
%
% Outputs:
%   data - The data stored in the binary file.  This is timestamps, range
%          data and optionally intensity data if that was stored in the 
%          file.
%   header - The header information stored in the file.
function [data,header] = read_ic4_data(filename)


    % First thing we do is read the laser header... this is also
    % a validity check on the file
    [header, skip_to_pos] = read_ic4_header(filename);
    
    % Next we need to open the file and skip ahead to the correct location
    % in the file
    fid = fopen(filename(filename~='"'));
    fseek(fid,skip_to_pos,'bof');
    
    % Initialize data depending on if intensity information is burried
    % in the file
    data.timestamps = zeros(1,header.num_scans);
    data.still_time = zeros(1,header.num_scans);
    data.euler = zeros(3,header.num_scans);
    data.quaternion = zeros(4,header.num_scans);
    data.compass_yaw = zeros(1,header.num_scans);
    data.angular_vel_body = zeros(3,header.num_scans);
    data.angular_vel_nav = zeros(3,header.num_scans);
    data.accel_body = zeros(3,header.num_scans);
    data.accel_nav = zeros(3,header.num_scans);
    data.velocity_nav = zeros(3,header.num_scans);
    data.angular_vel_raw = zeros(3,header.num_scans);
    data.mag_body = zeros(3,header.num_scans);
    data.temperature = zeros(1,header.num_scans);
    data.tracking_status = zeros(1,header.num_scans);
    
    % Read the data out of the file
    for i = 1:1:header.num_scans
       data.timestamps(i) = fread(fid,1,'float');
       data.still_time(i) = fread(fid,1,'float');
       data.euler(:,i) = fread(fid,3,'float'); 
       data.quaternion(:,i) = fread(fid,4,'float'); 
       data.compass_yaw(i) = fread(fid,1,'float'); 
       data.angular_vel_body(:,i) = fread(fid,3,'float'); 
       data.angular_vel_nav(:,i) = fread(fid,3,'float'); 
       data.accel_body(:,i) = fread(fid,3,'float'); 
       data.accel_nav(:,i) = fread(fid,3,'float'); 
       data.velocity_nav(:,i) = fread(fid,3,'float'); 
       data.angular_vel_raw(:,i) = fread(fid,3,'float'); 
       data.mag_body(:,i) = fread(fid,3,'float'); 
       data.temperature(i) = fread(fid,1,'float'); 
       data.tracking_status(i) = fread(fid,1,'uchar'); 
    end
    
    % Clean up
    fclose(fid);


end