function varargout = avalonGui(varargin)
% AVALONGUI M-file for avalonGui.fig
%      AVALONGUI, by itself, creates a new AVALONGUI or raises the existing
%      singleton*.
%
%      H = AVALONGUI returns the handle to a new AVALONGUI or the handle to
%      the existing singleton*.
%
%      AVALONGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in AVALONGUI.M with the given input arguments.
%
%      AVALONGUI('Property','Value',...) creates a new AVALONGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before avalonGui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to avalonGui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help avalonGui

% Last Modified by GUIDE v2.5 07-Mar-2010 09:33:17

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @avalonGui_OpeningFcn, ...
                   'gui_OutputFcn',  @avalonGui_OutputFcn, ...
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


% --- Executes just before avalonGui is made visible.
function avalonGui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to avalonGui (see VARARGIN)
% Clear console
clc;
% Choose default command line output for avalonGui
handles.output = hObject;

% set the evaluation panel invisible at the beginning
set(handles.evaluation_panel,'Visible','off');
set(handles.start_panel,'Visible','on');

global Stop;
Stop = 0;

% Init figures
backgroundImage = importdata('avalon.png');

axes(handles.axes_pic);
image(backgroundImage);
axis off
% Position
xlabel(handles.AxesPose,'x [m]');
ylabel(handles.AxesPose,'y [m]');
title(handles.AxesPose,'Boat Position');
grid(handles.AxesPose,'on');
axis(handles.AxesPose,'equal');

% Trajectory
xlabel(handles.AxesTrajectory,'long [m]');
ylabel(handles.AxesTrajectory,'lat [m]');
title(handles.AxesTrajectory,'Trajectory');
grid(handles.AxesTrajectory,'on');
axis(handles.AxesTrajectory,'equal');

% axis(handles.AxesTrajectory,[-world_size/2 world_size/2 -world_size/2 world_size/2]);

% Trajectory Waypoint 2
xlabel(handles.AxesTrajectoryWaypoint2,'long [m]');
ylabel(handles.AxesTrajectoryWaypoint2,'lat [m]');
title(handles.AxesTrajectoryWaypoint2,'Trajectory, Waypoints');

% Forces
xlabel(handles.AxesForces,'t [sec]');
ylabel(handles.AxesForces,'Force (N)');
% title(handles.AxesForce,'Total Forces');
grid(handles.AxesForces,'on');

% Sail Forces
xlabel(handles.AxesSailforce,'t [sec]');
ylabel(handles.AxesSailforce,'Sailforce (N)');
% title(handles.AxesForce,'Total Forces');
grid(handles.AxesSailforce,'on');

% Velocity
xlabel(handles.AxesVelocity,'t [sec]');
ylabel(handles.AxesVelocity,'Velocity [m/s]');
% title(handles.AxesVelocity,'Velocity');
grid(handles.AxesVelocity,'on');

% Desired Heading
compass(handles.AxesDesiredHeading, 0, 0);
title(handles.AxesDesiredHeading,'desired heading');

% Desired Heading
compass(handles.AxesPose3, 0, 0);
title(handles.AxesPose3,'avalon heading');

% State of Charge of Battery

% hAx = handles.AxesSoC;%('Position',[.1 .1 .1 0.6]);
% SoC = 70;
% hPatch = thermometer(hAx,[0 20 100],SoC);
% Update handles structure
guidata(hObject, handles);

% UIWAIT makes avalonGui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = avalonGui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


%  --- Executes on button press in simulate_pushbutton.
function simulate_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to simulate_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% global d_wind;
% global v_wind
% global alpha_rudder;
% global aoa_sail;
global Stop;
Stop = 0;

global T_sim;
T_sim = str2double(get(handles.t_sim,'String'));

% Clear figures
cla(handles.AxesPose);
cla(handles.AxesTrajectory);
cla(handles.AxesForces);
cla(handles.AxesVelocity);
cla(handles.AxesTrajectoryWaypoint2);
cla(handles.AxesSailforce);
cla(handles.AxesDesiredHeading);

% d_wind         = deg2rad(str2double(get(handles.wind_angle,'String')));
% v_wind         = str2double(get(handles.wind_speed,'String'));
% alpha_rudder   = deg2rad(str2double(get(handles.rudder_angle,'String')));
% aoa_sail       = deg2rad(str2double(get(handles.sail_angle,'String')));

%% run simulation

simulationOne;

%% open evaluation panel (make it visible) after the simulationOne

set(handles.evaluation_panel,'Visible','on');
set(handles.start_panel,'Visible','off');

% --- Executes on button press in stop_pushbutton.
function stop_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to stop_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Stop

Stop = 1;
disp('avalon stop cmd');

% Update handles structure
guidata(hObject, handles);


function sail_angle_Callback(hObject, eventdata, handles)
% hObject    handle to sail_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of sail_angle as text
%        str2double(get(hObject,'String')) returns contents of sail_angle as a double
d_sailValue = str2double(get(hObject,'String'));
% Update handles structure

 
%if user inputs something is not a number, or if the input is less than 0
%or greater than 100, then the slider value defaults to 0
if (isempty(d_sailValue) || d_sailValue < -90 || d_sailValue > 90)
    set(handles.slider_sailangle,'Value',0);
    set(handles.slider_sailangle,'String','0');
else
    set(handles.slider_sailangle,'Value',d_sailValue);
end

guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function sail_angle_CreateFcn(hObject, eventdata, handles)
% hObject    handle to sail_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function rudder_angle_Callback(hObject, eventdata, handles)
% hObject    handle to rudder_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rudder_angle as text
%        str2double(get(hObject,'String')) returns contents of rudder_angle as a double
d_rudderValue = str2double(get(hObject,'String'));
%if user inputs something is not a number, or if the input is less than 0
%or greater than 100, then the slider value defaults to 0
if (isempty(d_rudderValue) || d_rudderValue < -20 || d_rudderValue > 20)
    set(handles.slider_rudderangle,'Value',0);
    set(handles.slider_rudderangle,'String','0');
else
    set(handles.slider_rudderangle,'Value',d_rudderValue);
end

guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function rudder_angle_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rudder_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function wind_speed_Callback(hObject, eventdata, handles)
% hObject    handle to wind_speed (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of wind_speed as text
%        str2double(get(hObject,'String')) returns contents of wind_speed as a double
v_windValue = str2double(get(hObject,'String'));
% Update handles structure
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function wind_speed_CreateFcn(hObject, eventdata, handles)
% hObject    handle to wind_speed (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function wind_angle_Callback(hObject, eventdata, handles)
% hObject    handle to wind_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of wind_angle as text
%        str2double(get(hObject,'String')) returns contents of wind_angle as a double
d_windValue = str2double(get(hObject,'String'));
% Update handles structure
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function wind_angle_CreateFcn(hObject, eventdata, handles)
% hObject    handle to wind_angle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function t_sim_Callback(hObject, eventdata, handles)
% hObject    handle to t_sim (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of t_sim as text
%        str2double(get(hObject,'String')) returns contents of t_sim as a double
t_simValue = str2double(get(hObject,'String'));
% Update handles structure
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function t_sim_CreateFcn(hObject, eventdata, handles)
% hObject    handle to t_sim (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on slider movement.
function slider_sailangle_Callback(hObject, eventdata, handles)
% hObject    handle to slider_sailangle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

d_sailValue = get(handles.slider_sailangle,'Value');
 
%puts the slider value into the edit text component
set(handles.sail_angle,'String', num2str(d_sailValue));
 
% Update handles structure
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function slider_sailangle_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider_sailangle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on slider movement.
function slider_rudderangle_Callback(hObject, eventdata, handles)
% hObject    handle to slider_rudderangle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
d_rudderValue = get(handles.slider_rudderangle,'Value');
 
%puts the slider value into the edit text component
set(handles.rudder_angle,'String', num2str(d_rudderValue));
 
% Update handles structure
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function slider_rudderangle_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider_rudderangle (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on button press in close_window_pushbutton.
function close_window_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to close_window_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.evaluation_panel,'Visible','off');
set(handles.start_panel,'Visible','on');


function currentTime_Callback(hObject, eventdata, handles)
% hObject    handle to currentTime (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
currentTime = str2num(get(hObject,'String'));
% Hints: get(hObject,'String') returns contents of currentTime as text
%        str2double(get(hObject,'String')) returns contents of currentTime as a double


% --- Executes during object creation, after setting all properties.
function currentTime_CreateFcn(hObject, eventdata, handles)
% hObject    handle to currentTime (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on key press with focus on slider_rudderangle and none of its controls.
function slider_rudderangle_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to slider_rudderangle (see GCBO)
% eventdata  structure with the following fields (see UICONTROL)
%	Key: name of the key that was pressed, in lower case
%	Character: character interpretation of the key(s) that was pressed
%	Modifier: name(s) of the modifier key(s) (i.e., control, shift) pressed
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in manInChargeRadiobutton.
function manInChargeRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to manInChargeRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of manInChargeRadiobutton
man_in_charge = get(hObject,'Value');
% update
guidata(hObject, handles);



function AvalonSpeed_kn_Callback(hObject, eventdata, handles)
% hObject    handle to AvalonSpeed_kn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of AvalonSpeed_kn as text
%        str2double(get(hObject,'String')) returns contents of AvalonSpeed_kn as a double


% --- Executes during object creation, after setting all properties.
function AvalonSpeed_kn_CreateFcn(hObject, eventdata, handles)
% hObject    handle to AvalonSpeed_kn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function AvalonSpeed_ms_Callback(hObject, eventdata, handles)
% hObject    handle to AvalonSpeed_ms (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of AvalonSpeed_ms as text
%        str2double(get(hObject,'String')) returns contents of AvalonSpeed_ms as a double


% --- Executes during object creation, after setting all properties.
function AvalonSpeed_ms_CreateFcn(hObject, eventdata, handles)
% hObject    handle to AvalonSpeed_ms (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in plots_pushbutton.
function plots_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to plots_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.evaluation_panel,'Visible','on');
set(handles.start_panel,'Visible','off');
% update
guidata(hObjects,handles);


% --- Executes on button press in clear_pushbutton.
function clear_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to clear_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Clear figures
cla(handles.AxesPose);
cla(handles.AxesTrajectory);
cla(handles.AxesForces);
cla(handles.AxesVelocity);
cla(handles.AxesTrajectoryWaypoint2);
cla(handles.AxesSailforce);
cla(handles.AxesDesiredHeading);
% update
guidata(hObject,handles);


% --- Executes on button press in AisSignalRadiobutton.
function AisSignalRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to AisSignalRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of AisSignalRadiobutton


% --- Executes on button press in ImuSignalFreezeRadiobutton.
function ImuSignalFreezeRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to ImuSignalFreezeRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ImuSignalFreezeRadiobutton


% --- Executes on button press in ImuRandomWalkRadiobutton.
function ImuRandomWalkRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to ImuRandomWalkRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ImuRandomWalkRadiobutton


% --- Executes on button press in ImuBiasRadiobutton.
function ImuBiasRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to ImuBiasRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ImuBiasRadiobutton


% --- Executes on button press in ImuDriftRadiobutton.
function ImuDriftRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to ImuDriftRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ImuDriftRadiobutton


% --- Executes on button press in ImuPowerSupplyRadiobutton.
function ImuPowerSupplyRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to ImuPowerSupplyRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ImuPowerSupplyRadiobutton


% --- Executes on button press in GpsSignalFreezeRadiobutton.
function GpsSignalFreezeRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to GpsSignalFreezeRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of GpsSignalFreezeRadiobutton


% --- Executes on button press in GpsRandomWalkRadiobutton.
function GpsRandomWalkRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to GpsRandomWalkRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of GpsRandomWalkRadiobutton


% --- Executes on button press in GpsBiasRadiobutton.
function GpsBiasRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to GpsBiasRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of GpsBiasRadiobutton


% --- Executes on button press in GpsDriftRadiobutton.
function GpsDriftRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to GpsDriftRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of GpsDriftRadiobutton


% --- Executes on button press in GpsPowerSupplyRadiobutton.
function GpsPowerSupplyRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to GpsPowerSupplyRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of GpsPowerSupplyRadiobutton



function AisNumberOfBoats_Callback(hObject, eventdata, handles)
% hObject    handle to AisNumberOfBoats (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of AisNumberOfBoats as text
%        str2double(get(hObject,'String')) returns contents of AisNumberOfBoats as a double


% --- Executes during object creation, after setting all properties.
function AisNumberOfBoats_CreateFcn(hObject, eventdata, handles)
% hObject    handle to AisNumberOfBoats (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in emergencyStopRadiobutton.
function emergencyStopRadiobutton_Callback(hObject, eventdata, handles)
% hObject    handle to emergencyStopRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of emergencyStopRadiobutton



function flagsState_Callback(hObject, eventdata, handles)
% hObject    handle to flagsState (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of flagsState as text
%        str2double(get(hObject,'String')) returns contents of flagsState as a double


% --- Executes during object creation, after setting all properties.
function flagsState_CreateFcn(hObject, eventdata, handles)
% hObject    handle to flagsState (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function flagsStateReq_Callback(hObject, eventdata, handles)
% hObject    handle to flagsStateReq (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of flagsStateReq as text
%        str2double(get(hObject,'String')) returns contents of flagsStateReq as a double


% --- Executes during object creation, after setting all properties.
function flagsStateReq_CreateFcn(hObject, eventdata, handles)
% hObject    handle to flagsStateReq (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in SailStatePopupmenu.
function SailStatePopupmenu_Callback(hObject, eventdata, handles)
% hObject    handle to SailStatePopupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns SailStatePopupmenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from SailStatePopupmenu


% --- Executes during object creation, after setting all properties.
function SailStatePopupmenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to SailStatePopupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on slider movement.
function slider_rudderangleLeft_Callback(hObject, eventdata, handles)
% hObject    handle to slider_rudderangleLeft (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function slider_rudderangleLeft_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider_rudderangleLeft (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end



function rudder_angle_left_Callback(hObject, eventdata, handles)
% hObject    handle to rudder_angle_left (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rudder_angle_left as text
%        str2double(get(hObject,'String')) returns contents of rudder_angle_left as a double


% --- Executes during object creation, after setting all properties.
function rudder_angle_left_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rudder_angle_left (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function N_totEdit_Callback(hObject, eventdata, handles)
% hObject    handle to N_totEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of N_totEdit as text
%        str2double(get(hObject,'String')) returns contents of N_totEdit as a double


% --- Executes during object creation, after setting all properties.
function N_totEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to N_totEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function N_rudderEdit_Callback(hObject, eventdata, handles)
% hObject    handle to N_rudderEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of N_rudderEdit as text
%        str2double(get(hObject,'String')) returns contents of N_rudderEdit as a double


% --- Executes during object creation, after setting all properties.
function N_rudderEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to N_rudderEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function N_dampingEdit_Callback(hObject, eventdata, handles)
% hObject    handle to N_dampingEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of N_dampingEdit as text
%        str2double(get(hObject,'String')) returns contents of N_dampingEdit as a double


% --- Executes during object creation, after setting all properties.
function N_dampingEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to N_dampingEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function N_sailEdit_Callback(hObject, eventdata, handles)
% hObject    handle to N_sailEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of N_sailEdit as text
%        str2double(get(hObject,'String')) returns contents of N_sailEdit as a double


% --- Executes during object creation, after setting all properties.
function N_sailEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to N_sailEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function I_zEdit_Callback(hObject, eventdata, handles)
% hObject    handle to I_zEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of I_zEdit as text
%        str2double(get(hObject,'String')) returns contents of I_zEdit as a double


% --- Executes during object creation, after setting all properties.
function I_zEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to I_zEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function A_hull3Edit_Callback(hObject, eventdata, handles)
% hObject    handle to A_hull3Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of A_hull3Edit as text
%        str2double(get(hObject,'String')) returns contents of A_hull3Edit as a double


% --- Executes during object creation, after setting all properties.
function A_hull3Edit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to A_hull3Edit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in manInChargeRadiobutton.
function radiobutton26_Callback(hObject, eventdata, handles)
% hObject    handle to manInChargeRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of manInChargeRadiobutton



function edit21_Callback(hObject, eventdata, handles)
% hObject    handle to t_sim (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of t_sim as text
%        str2double(get(hObject,'String')) returns contents of t_sim as a double


% --- Executes during object creation, after setting all properties.
function edit21_CreateFcn(hObject, eventdata, handles)
% hObject    handle to t_sim (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in emergencyStopRadiobutton.
function radiobutton25_Callback(hObject, eventdata, handles)
% hObject    handle to emergencyStopRadiobutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of emergencyStopRadiobutton


% --- Executes on button press in simulate_pushbutton.
function pushbutton8_Callback(hObject, eventdata, handles)
% hObject    handle to simulate_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in stop_pushbutton.
function pushbutton9_Callback(hObject, eventdata, handles)
% hObject    handle to stop_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in plots_pushbutton.
function pushbutton10_Callback(hObject, eventdata, handles)
% hObject    handle to plots_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in clear_pushbutton.
function pushbutton11_Callback(hObject, eventdata, handles)
% hObject    handle to clear_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on selection change in SailStatePopupmenu.
function popupmenu3_Callback(hObject, eventdata, handles)
% hObject    handle to SailStatePopupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns SailStatePopupmenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from SailStatePopupmenu


% --- Executes during object creation, after setting all properties.
function popupmenu3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to SailStatePopupmenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in radiobutton20.
function radiobutton20_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton20 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton20


% --- Executes on button press in radiobutton21.
function radiobutton21_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton21 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton21


% --- Executes on button press in radiobutton22.
function radiobutton22_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton22 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton22


% --- Executes on button press in radiobutton23.
function radiobutton23_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton23 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton23


% --- Executes on button press in radiobutton24.
function radiobutton24_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton24 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton24


% --- Executes on button press in radiobutton15.
function radiobutton15_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton15 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton15


% --- Executes on button press in radiobutton16.
function radiobutton16_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton16 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton16


% --- Executes on button press in radiobutton17.
function radiobutton17_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton17 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton17


% --- Executes on button press in radiobutton18.
function radiobutton18_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton18 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton18


% --- Executes on button press in radiobutton19.
function radiobutton19_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton19 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton19



function Sail_factorEdit_Callback(hObject, eventdata, handles)
% hObject    handle to Sail_factorEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Sail_factorEdit as text
%        str2double(get(hObject,'String')) returns contents of Sail_factorEdit as a double


% --- Executes during object creation, after setting all properties.
function Sail_factorEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Sail_factorEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in Save_Plot_TrajectoryPushbutton.
function Save_Plot_TrajectoryPushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Save_Plot_TrajectoryPushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
savePlotWithinGUI(handles.AxesTrajectory);


% --- Executes on button press in Save_Plot_VelocityPushbutton.
function Save_Plot_VelocityPushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Save_Plot_VelocityPushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
savePlotWithinGUI(handles.AxesVelocity);

% --- Executes on button press in Save_Plot_ForcesPushbutton.
function Save_Plot_ForcesPushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Save_Plot_ForcesPushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
savePlotWithinGUI(handles.AxesForces);

% --- Executes on button press in Save_Plot_SailforcePushbutton.
function Save_Plot_SailforcePushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Save_Plot_SailforcePushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
legendSailforce = legend('F_x', 'F_y', 'M_z', 'Location', 'Best');
savePlotWithinGUI(handles.AxesSailforce,legendSailforce);

% --- Executes on button press in Save_Plot_TrajectoryWaypoint2Pushbutton.
function Save_Plot_TrajectoryWaypoint2Pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to Save_Plot_TrajectoryWaypoint2Pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
savePlotWithinGUI(handles.AxesTrajectoryWaypoint2);


% --- Executes on button press in radiobutton27.
function radiobutton27_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton27 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton27


% --- Executes on button press in radiobutton28.
function radiobutton28_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton28 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton28


% --- Executes on button press in radiobutton29.
function radiobutton29_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton29 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton29


% --- Executes on button press in radiobutton30.
function radiobutton30_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton30 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton30


% --- Executes on button press in radiobutton31.
function radiobutton31_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton31 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton31



function edit23_Callback(hObject, eventdata, handles)
% hObject    handle to edit23 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit23 as text
%        str2double(get(hObject,'String')) returns contents of edit23 as a double


% --- Executes during object creation, after setting all properties.
function edit23_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit23 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit24_Callback(hObject, eventdata, handles)
% hObject    handle to edit24 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit24 as text
%        str2double(get(hObject,'String')) returns contents of edit24 as a double


% --- Executes during object creation, after setting all properties.
function edit24_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit24 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in radiobutton32.
function radiobutton32_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton32 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton32



function heading_speed_Callback(hObject, eventdata, handles)
% hObject    handle to heading_speed (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of heading_speed as text
%        str2double(get(hObject,'String')) returns contents of heading_speed as a double


% --- Executes during object creation, after setting all properties.
function heading_speed_CreateFcn(hObject, eventdata, handles)
% hObject    handle to heading_speed (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in radiobutton34.
function radiobutton34_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton34 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton34


% --- Executes on button press in radiobutton35.
function radiobutton35_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton35 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton35


% --- Executes on button press in radiobutton36.
function radiobutton36_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton36 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton36


% --- Executes on button press in radiobutton37.
function radiobutton37_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton37 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton37


% --- Executes on button press in radiobutton38.
function radiobutton38_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton38 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton38



function edit26_Callback(hObject, eventdata, handles)
% hObject    handle to edit26 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit26 as text
%        str2double(get(hObject,'String')) returns contents of edit26 as a double


% --- Executes during object creation, after setting all properties.
function edit26_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit26 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in radiobutton33.
function radiobutton33_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton33 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton33



function edit27_Callback(hObject, eventdata, handles)
% hObject    handle to edit27 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit27 as text
%        str2double(get(hObject,'String')) returns contents of edit27 as a double


% --- Executes during object creation, after setting all properties.
function edit27_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit27 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


