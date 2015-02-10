function [ handles ] = plot_selected_mad( handles )
% This function handles the plotting of the selected .mad file from the
% folder contents box.

% Check to make sure a valid directory containing .MAD files is set.
if(isempty(handles.selected_dir))
   return 
end

% Get the value of the and string of the selected .mad File to view
selected_val  = get(handles.folder_contents_listbox,'Value');
selected_file = handles.files_in_dir(selected_val);

% Build Full File path for the File, and read the information from the file
fullpath = fullfile(handles.selected_dir, selected_file.name);
[poses] = readMAD(fullpath);

% Check to make sure the .mad File contained some info before attempting to
% plot
if(size(poses,1) ~= 6 || isempty(poses))
   return 
end

% Ensure the plot is visible
set(handles.main_axis,'Visible','on')

% Grab the color map or define order of colors.
C = get(gca,'ColorOrder');

% Plot the contents of the .mad file to the graph window
axis(handles.main_axis);
axis auto;
handles.timesplotted = handles.timesplotted + 1;
index = mod(handles.timesplotted,size(C,1)) + 1;
plot3(poses(1,:),poses(2,:),poses(3,:),'Color',C(index,:))
xlabel('X (m)'), ylabel('Y (m)'), zlabel('Z (m)')
axis equal;
view(0, 90);

% Add String to the active .MAD Files List
handles = addtofilesbox(handles, selected_file.name);

% Add to Structure containing active .mad files in handles
if(isempty(handles.active_mad{1}))
   handles.active_mad{1} = selected_file;
else
   handles.active_mad{end+1} = selected_file;
end

% Fix The Legend
legend off;
total_active = length(handles.active_mad);

files{total_active} = [];
for i = 1:1:total_active
    files{i} = handles.active_mad{i}.name;
end
handles.legend_plot1 = legend(files,'Location','NorthEast','Interpreter','None');
    
end