function [ handles ] = removedfromdirbox(handles, input )
% This function handles the removal of items from the directory listbox.
%
% input can either specify the number of the item you want to delete or the
% string 'all' which will empty the list.

% Grab the listbox strings
prev_str = get(handles.folder_contents_listbox,'String');

% Check for zero and singleton case
if(~iscell(prev_str))
    prev_str2 = char(32); 
    set(handles.folder_contents_listbox,'String',prev_str2,'Value',1);
    return
else
    if(ischar(input))
       if(strcmp(input,'all'))
           prev_str2 = char(32);
           set(handles.folder_contents_listbox,'String',prev_str2,'Value',1);
           return
       end
    else
           total_items = length(prev_str);
           
           % Check for erroneus numerical input
           if((input > total_items) ||( input < 1 ))
               return
           end
           
           % Remove selected input
           prev_str(input) = [];
           prev_str2 = prev_str;
           set(handles.folder_contents_listbox,'String',prev_str2,'Value',1);
           return
     end
end
end
