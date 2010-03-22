function varargout = avalonGuiWaypoints(varargin)
% AVALONGUIWAYPOINTS M-file for avalonGuiWaypoints.fig
%      AVALONGUIWAYPOINTS, by itself, creates a new AVALONGUIWAYPOINTS or raises the existing
%      singleton*.
%
%      H = AVALONGUIWAYPOINTS returns the handle to a new AVALONGUIWAYPOINTS or the handle to
%      the existing singleton*.
%
%      AVALONGUIWAYPOINTS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in AVALONGUIWAYPOINTS.M with the given input arguments.
%
%      AVALONGUIWAYPOINTS('Property','Value',...) creates a new AVALONGUIWAYPOINTS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before avalonGuiWaypoints_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to avalonGuiWaypoints_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help avalonGuiWaypoints

% Last Modified by GUIDE v2.5 08-Jan-2010 11:36:23

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @avalonGuiWaypoints_OpeningFcn, ...
                   'gui_OutputFcn',  @avalonGuiWaypoints_OutputFcn, ...
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


% --- Executes just before avalonGuiWaypoints is made visible.
function avalonGuiWaypoints_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to avalonGuiWaypoints (see VARARGIN)
clc;
% Choose default command line output for avalonGuiWaypoints
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes avalonGuiWaypoints wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = avalonGuiWaypoints_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in WaypointGeneration_Pushbutton.
function WaypointGeneration_Pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to WaypointGeneration_Pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

waypointGenerator;



function gridSize_Callback(hObject, eventdata, handles)
% hObject    handle to gridSize (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of gridSize as text
%        str2double(get(hObject,'String')) returns contents of gridSize as a double


% --- Executes during object creation, after setting all properties.
function gridSize_CreateFcn(hObject, eventdata, handles)
% hObject    handle to gridSize (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function n_waypoints_Callback(hObject, eventdata, handles)
% hObject    handle to n_waypoints (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of n_waypoints as text
%        str2double(get(hObject,'String')) returns contents of n_waypoints as a double


% --- Executes during object creation, after setting all properties.
function n_waypoints_CreateFcn(hObject, eventdata, handles)
% hObject    handle to n_waypoints (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function dist_Norm_Callback(hObject, eventdata, handles)
% hObject    handle to dist_Norm (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of dist_Norm as text
%        str2double(get(hObject,'String')) returns contents of dist_Norm as a double


% --- Executes during object creation, after setting all properties.
function dist_Norm_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dist_Norm (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function start_lat_Callback(hObject, eventdata, handles)
% hObject    handle to start_lat (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of start_lat as text
%        str2double(get(hObject,'String')) returns contents of start_lat as a double


% --- Executes during object creation, after setting all properties.
function start_lat_CreateFcn(hObject, eventdata, handles)
% hObject    handle to start_lat (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function start_long_Callback(hObject, eventdata, handles)
% hObject    handle to start_long (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of start_long as text
%        str2double(get(hObject,'String')) returns contents of start_long as a double


% --- Executes during object creation, after setting all properties.
function start_long_CreateFcn(hObject, eventdata, handles)
% hObject    handle to start_long (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


