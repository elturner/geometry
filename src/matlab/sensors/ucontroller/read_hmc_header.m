%
% read_hmc_header.m
% Nicholas Corso
%
% [header, file_pos_after_header] = read_hmc_header(filename)
%
% This function reads the header of the hmc data binary file and returns
% both the header and the file position that is the first byte after 
% the header.  
%
% For more information on the parameters see the hmc_dats.txt file in the
% filetypes folder of this project.
%
function [header, file_pos_after_header] = read_hmc_header(filename)

    % Attempt to open the file up in binary for reading
    fid = fopen(filename(filename ~= '"'),'rb');
    if(fid == -1) 
       error(['Unable to open file: ', filename]); 
    end
    
    header = [];
    
    % First we read in the magic number to ensure that this is a valid
    % file type to read from
    header.tag = fread(fid,9,'*char')';
    if(~strcmp(header.tag,['HMC5883L',char(0)]))
       fclose(fid);
       error(['Expected Magic Number ''HMC5883L\0'' found: ', header.tag, ...
          '\nIs this really a magnetometer .dat?']);
    end
    
    % The next thing in the header is the version number
    header.vermaj = fread(fid,1);
    header.vermin = fread(fid,1);
    
    % Then we read the size of the rest of the header
    header.header_metadata_size = fread(fid,1,'uint32');
    
    % Next we just straight out read the header data
    header.num_sensors = fread(fid,1,'uchar');
    header.num_readings = fread(fid,1,'uint32');
    header.gain = fread(fid,1,'double');
    header.conversion_to_seconds = fread(fid,1,'double');
    
    % Return where we left off in the file
    file_pos_after_header = ftell(fid);
    
    % Close the file and be done
    fclose(fid);

end
