%
% display_urg_file.m
% Nicholas Corso
%
% [figure_num] = display_urg_file(filename,figure_num)
%
% This function displays back the data from a urg binary file in real
% time to the user.
%
% Inputs:
%   filename - The name of the file you want to display
%   figure_num - An optional argument that throws the plot onto a specific
%                figure instead of creating a new one.  This will destroy
%                any already existing plot in figure figure_num
%
% Outputs:
%   figure_num - The number of the figure that was used for plotting
function [figure_num] = display_urg_file(filename,figure_num)

    % Read in the laser data from file
    [data,header] = read_urg_data(filename);
    
    % If no figure number was given create one
    if(~exist('figure_num','var') || isempty(figure_num))
       figure_num = figure; 
    else
       figure(figure_num); 
    end
    
    % Loop over the scans in file displaying them
    for i = 1:1:header.num_scans
        
       figure(figure_num)
       hold off;
       [x,y] = pol2cart(header.angle_map, data.range{i});
       if( header.capture_mode == 0 )
           scatter(x,y,'.'); axis equal;
       else
           scatter(x,y,3,data.intensity{i}), axis equal
       end
       
       % Calculate the realtime wait
       if(i ~= header.num_scans)
          wait_time = data.timestamps(i+1)-data.timestamps(i);
          wait_time = wait_time/1000;
       else
          wait_time = 0.1; 
       end
       drawnow
       pause(wait_time);
        
    end
    

end