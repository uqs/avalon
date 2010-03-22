function varargout = windfeld(varargin)
% WINDFELD M-file for windfeld.fig
%      WINDFELD, by itself, creates a new WINDFELD or raises the existing
%      singleton*.
%
%      H = WINDFELD returns the handle to a new WINDFELD or the handle to
%      the existing singleton*.
%
%      WINDFELD('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in WINDFELD.M with the given input arguments.
%
%      WINDFELD('Property','Value',...) creates a new WINDFELD or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before windfeld_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to windfeld_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help windfeld

% Last Modified by GUIDE v2.5 30-Oct-2009 10:00:36

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @windfeld_OpeningFcn, ...
                   'gui_OutputFcn',  @windfeld_OutputFcn, ...
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


% --- Executes just before windfeld is made visible.
function windfeld_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to windfeld (see VARARGIN)

% Choose default command line output for windfeld
handles.output = hObject;

global winddata;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes windfeld wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = windfeld_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in wind_pushbutton.
function wind_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to wind_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA);
cf=gcf;
windtest;

global winddata;
winddata.uneu=uneu;
winddata.vneu=vneu;
winddata.xneu=xneu;
winddata.yneu=yneu;
% set(handles.wind_pushbutton,winddata)


for i=1:41     % = 35
  for j=1:35     % = 41
 %     vneuI(i,:) = interp1(xneu(i,:),vneu(i,:),x_v);
 %      uneuI(:,j) = interp1(yneu(:,j),uneu(:,j),y_v);
      quiver(winddata.xneu(j,i), winddata.yneu(j,i), winddata.uneu(j,i), winddata.vneu(j,i), 0.05)    % generates arrows scalled by 0.05, uneuI, vneuI
       hold on
  end
end
guidata(hObject,handles)
  



% --- Executes during object creation, after setting all properties.
function windaxes_CreateFcn(hObject, eventdata, handles)
% hObject    handle to windaxes (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: place code in OpeningFcn to populate windaxes



function xpos_Callback(hObject, eventdata, handles)
% hObject    handle to xpos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of xpos as text
%        str2double(get(hObject,'String')) returns contents of xpos as a double


% --- Executes during object creation, after setting all properties.
function xpos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to xpos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit2_Callback(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit2 as text
%        str2double(get(hObject,'String')) returns contents of edit2 as a double


% --- Executes during object creation, after setting all properties.
function edit2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function ypos_Callback(hObject, eventdata, handles)
% hObject    handle to ypos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of ypos as text
%        str2double(get(hObject,'String')) returns contents of ypos as a double


% --- Executes during object creation, after setting all properties.
function ypos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to ypos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit4_Callback(hObject, eventdata, handles)
% hObject    handle to edit4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit4 as text
%        str2double(get(hObject,'String')) returns contents of edit4 as a double


% --- Executes during object creation, after setting all properties.
function edit4_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function d_wind_Callback(hObject, eventdata, handles)
% hObject    handle to d_wind (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of d_wind as text
%        str2double(get(hObject,'String')) returns contents of d_wind as a double


% --- Executes during object creation, after setting all properties.
function d_wind_CreateFcn(hObject, eventdata, handles)
% hObject    handle to d_wind (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function l_wind_Callback(hObject, eventdata, handles)
% hObject    handle to l_wind (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of l_wind as text
%        str2double(get(hObject,'String')) returns contents of l_wind as a double


% --- Executes during object creation, after setting all properties.
function l_wind_CreateFcn(hObject, eventdata, handles)
% hObject    handle to l_wind (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in wind_calculate_pushbutton.
function wind_calculate_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to wind_calculate_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
xpos=str2double(get(handles.xpos,'String'));
ypos=str2double(get(handles.ypos,'String'));

global winddata;
[d_wind, l_wind]=windstaerke(xpos,ypos,winddata.uneu,winddata.vneu);
d=num2str(d_wind);
l=num2str(l_wind);
set(handles.l_wind,'String',l);
set(handles.d_wind,'String',d);

