function [handles] = addtodirbox( handles , String )
%Adds item to the end of a listbox

% Grabs the list box strings
prev_str = get(handles.folder_contents_listbox,'String');

if(~iscell(prev_str))
    if(strcmp(prev_str,char(32)))
        prev_str2{1} = String;
    else
        prev_str2{1} = prev_str;
        prev_str2{2} = String;
    end
else
    prev_str{end+1} = String;
    prev_str2 = prev_str;
end
set(handles.folder_contents_listbox,'String',prev_str2,'Value',length(prev_str2));


end