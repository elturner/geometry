%
% read_gps_header.m
% Nicholas Corso
%
% [header, file_pos_after_header] = read_gps_header(filename)
%
% This function reads the header of the gps data binary file and returns
% both the header and the file position that is the first byte after 
% the header.
function [header, file_pos_after_header] = read_gps_header(filename)

    % Attempt to open the file up in binary for reading
    fid = fopen(filename(filename ~= '"'),'rb');
    if(fid == -1) 
       error(['Unable to open file: ', filename]); 
    end
    
    header = [];
    
    % First we read in the magic number to ensure that this is a valid
    % file type to read from
    header.tag = fread(fid,4,'*char')';
    if(~strcmp(header.tag,['GPS',char(0)]))
       fclose(fid);
       error(['Expected Magic Number ''GPS\0'' found: ', header.tag, ...
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
     
    % Then we read the size of the rest of the header
    header.header_metadata_size = fread(fid,1,'uint32');
    
    % Next we just straight out read the header data
    header.num_scans = fread(fid,1,'uint32');
    
    % Return where we left off in the file
    file_pos_after_header = ftell(fid);
    
    % Close the file and be done
    fclose(fid);

end