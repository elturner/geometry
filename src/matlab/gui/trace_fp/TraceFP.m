function varargout = TraceFP(varargin)
% TRACEFP MATLAB code for TraceFP.fig
%
%	This program allows a user to load, modify, and save .fp floorplan
%	files.
%
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
%      Written by:   Eric Turner <elturner@eecs.berkeley.edu>
%      Created on February 9, 2015
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help TraceFP

% Last Modified by GUIDE v2.5 10-Feb-2015 07:14:23

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @TraceFP_OpeningFcn, ...
                   'gui_OutputFcn',  @TraceFP_OutputFcn, ...
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


% --- Executes just before TraceFP is made visible.
function TraceFP_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to TraceFP (see VARARGIN)

	clc;
	fprintf('[TraceFP]\tHello!\n');

	% initialize handles structure
	handles.wall_samples = []; % no wall samples yet
	handles.control_points = zeros(0,2); 
			% no control points (each row (x,y))
	handles.triangles = zeros(0,3); 
			% no polygons yet, each indexes 3 ctrl pts
	handles.room_ids     = []; % one per triangle
	handles.current_room = 1;


	% initialize plot handles
	handles.wall_samples_plot = 0; % not valid handle
	handles.control_points_plot = 0;
	handles.triangles_plot = 0;
	
	% initialize GUI
	set(handles.show_wall_samples, 'Value', 1);
	set(handles.show_control_points, 'Value', 1);
	set(handles.show_floorplan, 'Value', 1);

	% Choose default command line output for TraceFP
	handles.output = hObject;

	% Update handles structure
	guidata(hObject, handles);

	% UIWAIT makes TraceFP wait for user response (see UIRESUME)
	%uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = TraceFP_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% Get default command line output from handles structure
	%varargout{1} = handles.output;


% --- Executes on button press in show_wall_samples.
function show_wall_samples_Callback(hObject, eventdata, handles)
% hObject    handle to show_wall_samples (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% Hint: get(hObject,'Value') returns toggle state 
	% of show_wall_samples
	if(handles.wall_samples_plot == 0)
		fprintf('[TraceFP]\tNo wall samples defined.\n');
	elseif(get(handles.show_wall_samples, 'Value'))
		fprintf('[TraceFP]\tShow wall samples\n');
		set(handles.wall_samples_plot, 'Visible', 'on');
	else
		fprintf('[TraceFP]\tHide wall samples\n');
		set(handles.wall_samples_plot, 'Visible', 'off');
	end

% --- Executes on button press in show_control_points.
function show_control_points_Callback(hObject, eventdata, handles)
% hObject    handle to show_control_points (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% Hint: get(hObject,'Value') returns toggle state 
	% of show_control_points
	if(handles.control_points_plot == 0)
		fprintf('[TraceFP]\tNo control points defined.\n');
	elseif(get(handles.show_control_points, 'Value'))
		fprintf('[TraceFP]\tShow control points\n');
		set(handles.control_points_plot, 'Visible', 'on');
	else
		fprintf('[TraceFP]\tHide control points\n');
		set(handles.control_points_plot, 'Visible', 'off');
	end


% --- Executes on button press in show_floorplan.
function show_floorplan_Callback(hObject, eventdata, handles)
% hObject    handle to show_floorplan (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% Hint: get(hObject,'Value') returns toggle state of show_floorplan
	if(handles.triangles_plot == 0)
		fprintf('[TraceFP]\tNo triangles defined.\n');
	elseif(get(handles.show_floorplan, 'Value'))
		fprintf('[TraceFP]\tShow triangles\n');
		set(handles.triangles_plot, 'Visible', 'on');
	else
		fprintf('[TraceFP]\tHide triangles\n');
		set(handles.triangles_plot, 'Visible', 'off');
	end


% --- Executes on button press in new_polygon.
function new_polygon_Callback(hObject, eventdata, handles)
% hObject    handle to new_polygon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tNew triangle:  select three points...\n');

	% get the point indices
	pinds = zeros(1,3);
	for i = 1:3
		pinds(i) = TraceFP_select(handles);
	end

	% check for invalid
	if(length(unique(pinds)) < 3 || any(pinds <= 0))
		fprintf(['[TraceFP]\t\tFound repeated/invalid points, ', ...
				'discarding selection.\n']);
		return;
	end

	% check if triangle oriented correctly
	orient = det([ 	(handles.control_points(pinds(1),:) ...
				- handles.control_points(pinds(3),:)) ;
			(handles.control_points(pinds(2),:) ...
				- handles.control_points(pinds(3),:)) ]);
	if(orient < 0)
		fprintf('[TraceFP]\t\treordering to be counterclockwise\n');
		pinds = fliplr(pinds);
	end

	% add this triangle
	handles.triangles = [handles.triangles; pinds];
	fprintf('[TraceFP]\t\tadded new triangle\n');
	
	% update rendering
	handles.room_ids = [handles.room_ids ; handles.current_room];
	if(handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end

	% render and save data
	TraceFP_render(hObject, handles);



% --- Executes on button press in remove_polygon.
function remove_polygon_Callback(hObject, eventdata, handles)
% hObject    handle to remove_polygon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tremove polygon...\n');
	ind = TraceFP_findtri(handles);
	if(ind <= 0)
		return; % no triangle specified
	end

	% get the triangle and delete it
	handles.triangles(ind,:) = [];
	handles.room_ids(ind) = [];

	% update rendering
	if(handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end

	% render and save data
	TraceFP_render(hObject, handles);


% --- Executes on button press in set_room.
function set_room_Callback(hObject, eventdata, handles)
% hObject    handle to set_room (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tset current room...\n');

	% select a triangle
	ind = TraceFP_findtri(handles);
	if(ind <= 0)
		% no triangle selected.  Do they want a new room?
		answer = questdlg(['No reference triangle selected.  ', ...
			'Do you want to define a new room?'], 'New Room?');
		if(~strcmp(answer, 'Yes'))
			fprintf('[TraceFP]\t\tCancelling...\n');
			return;
		end

		% make a new room
		handles.current_room = 1 + max(handles.room_ids);
		fprintf('[TraceFp]\t\tNew room: %d\n', ...
				handles.current_room);
	else
		% make current room index to be the room of 
		% the selected triangle
		handles.current_room = handles.room_ids(ind);
		fprintf('[TraceFP]\t\tCurrent room: %d\n', ...
				handles.current_room);
	end

	% update gui data
	guidata(hObject, handles);



% --- Executes on button press in new_point.
function new_point_Callback(hObject, eventdata, handles)
% hObject    handle to new_point (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% get new point
	fprintf('[TraceFP]\tnew point...\n');
	[X,Y] = ginput(1);

	% add to figure
	handles.control_points = [handles.control_points; X Y];
	if(handles.control_points_plot ~= 0)
		delete(handles.control_points_plot);
		handles.control_points_plot = 0;
	end

	% render and save data
	TraceFP_render(hObject, handles);



% --- Executes on button press in move_point.
function move_point_Callback(hObject, eventdata, handles)
% hObject    handle to move_point (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% tell user what's going on
	fprintf('[TraceFP]\tmove point...\n');

	% use selection tool to find a point
	pind = TraceFP_select(handles);
	if(pind <= 0)
		return;
	end

	% now ask the user to click a new spot
	fprintf('[TraceFP]\t\tSelect new location\n');
	[X,Y] = ginput(1);

	% set point to that location
	handles.control_points(pind, :) = [X, Y];

	% redraw
	if(handles.control_points_plot ~= 0)
		delete(handles.control_points_plot);
		handles.control_points_plot = 0;
	end
	if(any(handles.triangles(:) == pind) && handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end
	TraceFP_render(hObject, handles);
	fprintf('[TraceFP]\t\tpoint moved\n');



% --- Executes on button press in remove_point.
function remove_point_Callback(hObject, eventdata, handles)
% hObject    handle to remove_point (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	% tell user what we're doing
	fprintf('[TraceFP]\tremove point...\n');

	% use selection tool to find a point
	pind = TraceFP_select(handles);
	if(pind <= 0)
		return;
	end

	% remove triangles that contain this point
	to_remove = any( handles.triangles == pind, 2);
	handles.triangles( to_remove , :) = [];
	handles.room_ids( to_remove ) = [];

	% update the indexing in remaining triangles
	idx = [[1:pind] [pind:size(handles.control_points,1)]];
	handles.triangles = idx( handles.triangles );

	% remove this point from our list of points
	handles.control_points(pind, :) = [];

	% redraw
	if(handles.control_points_plot ~= 0)
		delete(handles.control_points_plot);
		handles.control_points_plot = 0;
	end
	if( sum(to_remove) > 0 && handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end
	TraceFP_render(hObject, handles);
	fprintf('[TraceFP]\t\tpoint removed\n');


% --------------------------------------------------------------------
function open_wall_samples_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to open_wall_samples (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tOpen Wall Samples...\n');

	% get the file path
	[dqfile, pathname, success] = uigetfile('*.dq','Open Wall Samples');
	if(~success)
		fprintf('[TraceFP]\t\tNevermind.\n');
		return;
	end

	% load it
	fprintf('[TraceFP]\t\tloading (this may take a while)...\n');
	handles.wall_samples = readMapData(fullfile([pathname, dqfile]));

	% render it
	if(handles.wall_samples_plot ~= 0)
		delete(handles.wall_samples_plot);
		handles.wall_samples_plot = 0;
	end
	TraceFP_render(hObject, handles);
	fprintf('[TraceFP]\t\tDONE\n.');


% --------------------------------------------------------------------
function save_fp_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to save_fp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tSave Floorplan file...\n');

	% ask user for a file
	[fpfile, pathname, success] = uiputfile(...
		{'*.fp', 'Floorplan files (*.fp)'}, 'Save floorplan as');
	if(~success)
		fprintf('[TraceFP]\t\tnevermind.\n');
		return;
	end

	% populate floorplan structure
	num_rooms = length(unique(handles.room_ids));
	floorplan = struct('res', 0, ...
			'num_verts', size(handles.control_points,1),...
			'verts', handles.control_points, ...
			'num_tris', size(handles.triangles,1), ...
			'tris', handles.triangles, ...
			'num_rooms', num_rooms, ...
			'room_inds', handles.room_ids, ...
			'room_floors', zeros(num_rooms, 1), ...
			'room_ceilings', 3*ones(num_rooms, 1));

	% write out file
	fprintf('[TraceFP]\t\twriting file (this may take a while) ...\n');
	write_fp(fullfile([pathname, fpfile]), floorplan);
	fprintf('[TraceFP]\t\tDONE\n');




% --- Executes on button press in update_triangle_room.
function update_triangle_room_Callback(hObject, eventdata, handles)
% hObject    handle to update_triangle_room (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tUpdate triangle room...\n');
	
	% find a triangle
	ind = TraceFP_findtri(handles);
	if(ind <= 0)
		fprintf('[TraceFP]\t\tNo triangle selected.  Nevermind.\n');
		return;
	end

	% change its room to current
	handles.room_ids(ind) = handles.current_room;

	% update
	if(handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end

	TraceFP_render(hObject, handles);
	fprintf('[TraceFP]\t\tUpdated to %d.\n', handles.current_room);


% --------------------------------------------------------------------
function open_fp_ClickedCallback(hObject, eventdata, handles)
% hObject    handle to open_fp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

	fprintf('[TraceFP]\tOpen floorplan (*.fp) file...\n');
	
	% open the file
	[fpfile, pathname,success] = uigetfile('*.fp', ...
			'Open Existing Floorplan');
	if(~success)
		fprintf('[TraceFP]\t\tNevermind.\n');
		return;
	end
	confirm = questdlg(...
		['Loading a floorplan will erase any existing data. ',...
		'Do you wish to proceed?'], 'Confirm');
	if(~strcmp(confirm, 'Yes'))
		fprintf('[TraceFP]\t\tCancelling import\n');
		return;
	end

	% load it
	fprintf('[TraceFP]\t\tloading (this may take a while)...\n');
	fp = read_fp(fullfile([pathname, fpfile]));
	handles.control_points = fp.verts;
	handles.triangles      = fp.tris;
	handles.room_ids       = fp.room_inds;
	handles.current_room   = 1;

	% render it
	if(handles.control_points_plot ~= 0)
		delete(handles.control_points_plot);
		handles.control_points_plot = 0;
	end
	if(handles.triangles_plot ~= 0)
		delete(handles.triangles_plot);
		handles.triangles_plot = 0;
	end

	TraceFP_render(hObject, handles);
	fprintf('[TraceFP]\t\tDONE\n.');

