function [ handles ] = removedfromfilesbox(handles, input )
% This function handles the removal of items from the files listbox.
%
% input can either specify the number of the item you want to delete or the
% string 'all' which will empty the list.

% Grab the listbox strings
prev_str = get(handles.files_listbox,'String');

% Check for zero and singleton case
if(~iscell(prev_str))
    prev_str2 = char(32); 
    set(handles.files_listbox,'String',prev_str2,'Value',1);
    
    % Check for singleton
    if(length(handles.active_mad) == 1)
       children = get(handles.main_axis,'children');
       delete(children);
       handles.active_mad = [];
       handles.active_mad{1} = [];
       
       % Fix The Legend
       legend off;
       set(handles.main_axis,'Visible','off')
    end
      
    return
else
    if(ischar(input))
       % Handle All Case
       if(strcmp(input,'all'))
           children = get(handles.main_axis,'children');
           delete(children);
           handles.active_mad = [];
           handles.active_mad{1} = [];
           prev_str2 = char(32); 
           set(handles.files_listbox,'String',prev_str2,'Value',1);
           % Fix The Legend
           legend off;
           set(handles.main_axis,'Visible','off')
           return
       end
    else
           total_items = length(prev_str);
           
           % Check for erroneus numerical input
           if((input > total_items) ||( input < 1 ))
               return
           end
           
           % Remove selected input
           children = get(handles.main_axis,'children');
           delete(children(end-input+1));
           
           % Hide Axis if no plots
           if(size(children,1)==1)
              set(handles.main_axis,'Visible','off') 
           end
           
           % Get Rid of Extra Axis Space
           axis auto;
           axis equal;
           
           % Fix List
           prev_str(input) = [];
           prev_str2 = prev_str;
           
           %Find next Value of listbox
           if (input == 1)
                out = input;
           else
               out = input - 1;
           end
           set(handles.files_listbox,'String',prev_str2,'Value',(out));
           
           % Remove from list of active files
           handles.active_mad(input) = [];
           
           % Fix The Legend
           legend off;
           total_active = length(handles.active_mad);
           
           if(total_active ~= 0)
               files{total_active} = [];
               for i = 1:1:total_active
                   files{i} = handles.active_mad{i}.name;
               end
               legend(files,'Location','NorthEast')
           end
          
           % If Structure is Empty Recreate it
           if(isempty(handles.active_mad))
              handles.active_mad{1} = []; 
           end
           return
     end
end
end
