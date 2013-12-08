%
% read_bmp_header.m
% Nicholas Corso
%
% [header, file_pos_after_header] = read_bmp_header(filename)
%
% This function reads the header of the hmc data binary file and returns
% both the header and the file position that is the first byte after 
% the header.
%
% For more information about the fields of the header please see the
% bmp_dats.txt file in the filetypes folder of this project.
%
function [header, file_pos_after_header] = read_bmp_header(filename)

    % Attempt to open the file up in binary for reading
    fid = fopen(filename(filename ~= '"'),'rb');
    if(fid == -1) 
       error(['Unable to open file: ', filename]); 
    end
    
    header = [];
    
    % First we read in the magic number to ensure that this is a valid
    % file type to read from
    header.tag = fread(fid,7,'*char')';
    if(~strcmp(header.tag,['BMP085',char(0)]))
       fclose(fid);
       error(['Expected Magic Number ''BMP085\0'' found: ', header.tag, ...
          '\nIs this really a barometer .dat?']);
    end
    
    % The next thing in the header is the version number
    header.vermaj = fread(fid,1);
    header.vermin = fread(fid,1);
    
    % Then we read the size of the rest of the header
    header.header_metadata_size = fread(fid,1,'uint32');
    
    % Next we just straight out read the header data
    header.calib_coeffs = zeros(1,11);
    header.calib_coeffs(1:3) = fread(fid,3,'int16','b');
    header.calib_coeffs(4:6) = fread(fid,3,'uint16','b');
    header.calib_coeffs(7:11) = fread(fid,5,'int16','b');
    header.num_readings = fread(fid,1,'uint32');
    header.oversampling = fread(fid,1,'uchar');
    header.conversion_to_seconds = fread(fid,1,'double');
    
    % Return where we left off in the file
    file_pos_after_header = ftell(fid);
    
    % Close the file and be done
    fclose(fid);

end