function [ handles ] = viewMAD_setdir(handles)
% This function handles selecting a directory in which to search for .mad
% files and then populate the listbox with the .mad files in that
% directory.  Allows selectivity of the MAD

% Allow the user to select a directroy in which to search for .mad files
directory = uigetdir;

% Error handling of uigetdir
if(isnumeric(directory) || exist(char(directory)) == 0) %#ok<EXIST>
    set(handles.folder_contents_listbox,'String',char(32),'Value',1)
    handles = addtodirbox(handles,'Error Selecting Directory');
    handles.files_in_dir = [];
    handles.selected_dir = [];
    return;
end

handles.selected_dir = directory;

% Update MAD File Listbox
handles = viewMAD_updateDir(handles);

end

