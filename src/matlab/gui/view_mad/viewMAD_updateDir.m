function [ handles ] = viewMAD_updateDir(handles)
% This function handles updating the mad file listbox. 
% Allows selectivity of the MAD

% Allow the user to select a directroy in which to search for .mad files
directory = handles.selected_dir;

% Clear Directory Listbox
handles = removedfromdirbox(handles,'all');

% Search the directory and then filter it for files with the .mad extension
files = dir(directory);
numFiles = size(files,1);
index = 1;

% Trim the struct of files without .mad extensions
while(index <= numFiles)
   if(length(files(index).name) < 4)
      files(index) = [];
      numFiles = numFiles - 1;
   elseif(~strcmp(files(index).name(end-3:end),'.mad'))
      files(index) = []; 
      numFiles = numFiles - 1;
   else
      index = index + 1; 
   end
    
end

% Set the variables to refer to the fact no mad files are in the folder
if(isempty(files))
   handles = addtodirbox(handles,'No .mad Files Found');
   handles.files_in_dir = [];
   handles.selected_dir = [];
   return
end

% Add the .mad Files to the listbox
for i = 1:1:numFiles
   handles = addtodirbox(handles,files(i).name);
end

handles.files_in_dir = files;
handles.selected_dir = directory;

end

