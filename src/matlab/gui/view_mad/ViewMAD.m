function varargout = ViewMAD(varargin)
% VIEWMAD M-file for ViewMAD.fig
%      VIEWMAD, by itself, creates a new VIEWMAD or raises the existing
%      singleton*.
%
%      H = VIEWMAD returns the handle to a new VIEWMAD or the handle to
%      the existing singleton*.
%
%      VIEWMAD('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in VIEWMAD.M with the given input arguments.
%
%      VIEWMAD('Property','Value',...) creates a new VIEWMAD or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before ViewMAD_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to ViewMAD_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES
%% CREATION FUNCTION OF THE GUI
% Edit the above text to modify the response to help ViewMAD

% Last Modified by GUIDE v2.5 21-Dec-2011 16:18:13

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @ViewMAD_OpeningFcn, ...
                   'gui_OutputFcn',  @ViewMAD_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before ViewMAD is made visible.
function ViewMAD_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to ViewMAD (see VARARGIN)
%% Command That Run at Startup
% Choose default command line output for ViewMAD
handles.output = hObject;

% Default Settings for the Axis

% Allocate extra fields
handles.files_in_dir = [];
handles.selected_dir = [];   
handles.active_mad{1} = [];
handles.timesplotted = 0;

% Kill Hung Files In Listbox graph etc
set(handles.folder_contents_listbox,'String',char(32),'Value',1)
handles = removedfromfilesbox(handles, 'all');

if length(varargin) == 2 && strcmp(varargin{1}, 'dir')
    handles.selected_dir = varargin{2};
    handles = viewMAD_updateDir(handles);
end

% Visibility off by Default
set(handles.main_axis,'Visible','off')

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes ViewMAD wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = ViewMAD_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
%% OUTPUT HANDLING

% --- Executes on selection change in files_listbox.
function files_listbox_Callback(hObject, eventdata, handles) %#ok<*INUSD,*DEFNU>
% hObject    handle to files_listbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns files_listbox contents as cell array
%        contents{get(hObject,'Value')} returns selected item from files_listbox
%% LISTBOX CALLBACK

% --- Executes during object creation, after setting all properties.
function files_listbox_CreateFcn(hObject, eventdata, handles)
% hObject    handle to files_listbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
%% LISTBOX CREATE FUNCTION
% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in plot_button.
function plot_button_Callback(hObject, eventdata, handles)
% hObject    handle to plot_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% PLOT BUTTON CALLBACK
% This function plots the madfile to the axis.

% Makes the main_axis box the current axis for plotting.
axes(handles.main_axis)

% Keep previous inputs
hold on;

% Plotting The Function
handles = plot_selected_mad(handles);

guidata(hObject, handles);


% --- Executes on button press in delete_button.
function delete_button_Callback(hObject, eventdata, handles)
% hObject    handle to delete_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% DELETE BUTTON CALLBACK
input = get(handles.files_listbox,'Value');

handles = removedfromfilesbox(handles, input);

guidata(hObject, handles)

% --- Executes on button press in clear_button.
function clear_button_Callback(hObject, eventdata, handles)
% hObject    handle to clear_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% CLEAR BUTTON CALLBACK

handles = removedfromfilesbox(handles, 'all');

guidata(hObject, handles)

% --- Executes on selection change in folder_contents_listbox.
function folder_contents_listbox_Callback(hObject, eventdata, handles)
% hObject    handle to folder_contents_listbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns folder_contents_listbox contents as cell array
%        contents{get(hObject,'Value')} returns selected item from folder_contents_listbox


% --- Executes during object creation, after setting all properties.
function folder_contents_listbox_CreateFcn(hObject, eventdata, handles)
% hObject    handle to folder_contents_listbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --------------------------------------------------------------------
function toolbar_setdir_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to toolbar_setdir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% CONTROL OF THE SET DIRECTORY TOOLBAR BUTTON
handles = viewMAD_setdir(handles);

guidata(hObject, handles);

% --------------------------------------------------------------------
function toolbar_saveFigure_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to toolbar_setdir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% CONTROL OF THE SET DIRECTORY TOOLBAR BUTTON
viewMAD_saveFigure(handles.main_axis, handles.legend_plot1);

guidata(hObject, handles);


% --------------------------------------------------------------------
function debug_menu_Callback(hObject, eventdata, handles)
% hObject    handle to debug_menu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%% DEBUG MENU 
% When debug is activated Ctrl+D will run this function.

handles
guidata(hObject, handles);


% --- Executes on button press in refreshDirButton.
function refreshDirButton_Callback(hObject, eventdata, handles)
% hObject    handle to refreshDirButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles = viewMAD_updateDir(handles);
guidata(hObject, handles);
