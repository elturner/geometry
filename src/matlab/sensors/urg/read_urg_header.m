%
% read_urg_header.m
% Nicholas Corso
%
% [header, file_pos_after_header] = read_urg_header(filename)
%
% This function reads the header of the urg data binary file and returns
% both the header and the file position that is the first byte after 
% the header.
function [header, file_pos_after_header] = read_urg_header(filename)

    % Attempt to open the file up in binary for reading
    fid = fopen(filename(filename ~= '"'),'rb');
    if(fid == -1) 
       error(['Unable to open file: ', filename]); 
    end
    
    header = [];
    
    % First we read in the magic number to ensure that this is a valid
    % file type to read from
    header.tag = fread(fid,6,'*char')';
    if(~strcmp(header.tag,['LASER',char(0)]))
       fclose(fid);
       error(['Expected Magic Number ''LASER\0'' found: ', header.tag, ...
          '\nIs this really a laser .dat?']);
    end
    
    % The next thing in the header is the version number
    header.vermaj = fread(fid,1);
    header.vermin = fread(fid,1);
    
    % Read the model number
    count = 1;
    mn = [];
    while(count ~= 0)
       [readchar, count] = fread(fid,1,'*char');
       mn = [mn, readchar];
       if(readchar == char(0))
          break 
       end
    end
    header.model_string = mn;
    
    
    % Then we read in the serial number
    count = 1; 
    sn = [];
    while(count ~= 0)
       [readchar, count] = fread(fid,1,'*char');
       sn = [sn, readchar];
       if(readchar == char(0))
          break 
       end
    end
    header.serial_number = sn;
    
    % Then we read the size of the rest of the header
    header.header_metadata_size = fread(fid,1,'uint32');
    
    % Next we just straight out read the header data
    header.capture_mode = fread(fid,1,'int32');
    header.num_scans = fread(fid,1,'uint32');
    header.points_per_scan = fread(fid,1,'int32');
    header.max_range = fread(fid,1,'int32');
    header.min_range = fread(fid,1,'int32');
    header.angle_map = zeros(header.points_per_scan,1);
    for i = 1:1:header.points_per_scan
       header.angle_map(i) = fread(fid,1,'float');  
    end
    
    % Return where we left off in the file
    file_pos_after_header = ftell(fid);
    
    % Close the file and be done
    fclose(fid);

end