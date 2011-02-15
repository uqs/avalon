#############################################################################
#                                                                           #
#                       P R O J E C T    A V A L O N                        #
#                                                                           #
#      		       Script for AVALON-GUI version 1.0 		    #
#                                                                           #
#                                                                           #
#        (c) Gion-Andri Buesser  -  gbuesser@student.ethz.ch                #
#                                                                           #
#############################################################################

####import modules: ####
import wxversion
wxversion.ensureMinimal('2.8')

import wx
#import time	
import sys
import os
sys.path.append('/usr/local/lib/python2.6/site-packages/')
import ddxInterface
import wx.gizmos as gizmos
import math


#---------------------------------------
#for plotting
#---------------------------------------
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas
from matplotlib.backends.backend_wx import NavigationToolbar2Wx
from matplotlib.figure import Figure


from numpy import *
#import numpy as np
import pylab
#import Gnuplot, Gnuplot.funcutils
 


EARTH_RAD = 6371000.1

#### main frame: ####
class ControlFrame(wx.Frame):
	def __init__(self, parent, id, title):
		wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(640, 500))

		self.initdone = 0 
		self.create_menu()
		#-------------------------------------------
		##### Create Sizers and Panels: #####
		#-------------------------------------------
		mainbox = wx.BoxSizer(wx.VERTICAL)
		perfbox = wx.BoxSizer(wx.HORIZONTAL)
		rightbox = wx.BoxSizer(wx.VERTICAL)
		firstbox = wx.BoxSizer(wx.HORIZONTAL)

		flagpanel = wx.Panel(self, -1, size=(500,110))
		envpanel = wx.Panel(self, -1, size=(230,120))
		speedpanel = wx.Panel(self, -1, size=(230,100))
		energypanel = wx.Panel(self, -1, size=(230,90))
		# panel colors:
		#envpanel.SetBackgroundColour('green')
		#speedpanel.SetBackgroundColour('yellow')
		#flagpanel.SetBackgroundColour('blue')
		#energypanel.SetBackgroundColour('magenta')

		#-------------------------------------------
		##### Set Timer: #####
		#-------------------------------------------
		self.timer = wx.Timer(self)
		self.Bind(wx.EVT_TIMER, self.OnUpdate, self.timer)

		fatfont = wx.Font(10, wx.DEFAULT, wx.NORMAL, wx.BOLD)
		title = wx.StaticText(self, 1, 'Control Panel')
		title.SetFont(fatfont)
		line = wx.StaticLine(self, -1,(10,10),(560,1))
		#line3 = wx.StaticLine(self, -1,(10,10),(560,1))

		mainbox.Add((0,20),0)
		mainbox.Add(title, 0, wx.ALIGN_CENTER, 20)
		mainbox.Add(line,0,wx.ALIGN_CENTER | wx.ALL ,10)

	
		#-------------------------------------------
		##### Fill Panels: #####
		#-------------------------------------------
		#self.fill_flagpanel(flagpanel)
		#self.fill_envpanel(envpanel)
		#self.fill_speedpanel(speedpanel)
		#self.fill_energypanel(energypanel)


		##### Add to sizers: #####
		perfbox.Add(speedpanel)

		rightbox.Add(envpanel)
		rightbox.Add(energypanel)
		firstbox.Add(flagpanel, 0, wx.ALIGN_CENTER)
		firstbox.Add(rightbox, 0, wx.ALIGN_CENTER)

		#mainbox.Add(firstbox,0,wx.ALIGN_CENTER | wx.RIGHT | wx.LEFT, 15)
		#mainbox.Add(line3,0,wx.ALIGN_CENTER | wx.TOP ,5)

		#-----------------------------------------------------------
		##### Create Plots:
		#-----------------------------------------------------------
		self.speeddata = [0]
		self.pos_longitude = []
		self.pos_latitude = []
		self.wyp_longitude = [0.0]
		self.wyp_latitude = [0.0]
		self.logtime = [0]

		self.plot_count = 0

		#self.create_speed_plot()
		self.create_wyp_plot()

		#perfbox.Add(self.speed_panel, 1, wx.TOP | wx.LEFT | wx.GROW, 10)
		#mainbox.Add(perfbox, 0, wx.ALIGN_CENTER | wx.ALL , 20)

		


		mainbox.Add(self.canvas, 1, wx.CENTER)

	
		self.SetSizer(mainbox)

	def fill_energypanel(self, energypanel):	
		energybox = wx.StaticBox(energypanel, -1, 'Energy Management', (5,5), size=(220,60))

		wx.StaticText(energypanel, -1, 'Voltage:', pos=(20,30))
		self.voltage = wx.TextCtrl(energypanel, 46, 'DEFAULT', pos=(110,26), size=(60,-1))
		wx.StaticText(energypanel, -1, 'Volts', pos=(175,30))

	def fill_envpanel(self, envpanel):
		envbox = wx.StaticBox(envpanel, -1, 'Environment', (5,5), size=(220,95))

		wx.StaticText(envpanel, -1, 'Wind Speed:', pos=(20,30))
		self.wind_speed = wx.TextCtrl(envpanel, 44, 'DEFAULT', pos=(110,26), size=(60,-1))
		wx.StaticText(envpanel, -1, 'knots', pos=(175,30))
		wx.StaticText(envpanel, -1, 'Direction:', pos=(20,60))
		self.wind_direction = wx.TextCtrl(envpanel, 45, 'DEFAULT', pos=(110,56), size=(60,-1))
		wx.StaticText(envpanel, -1, 'degree', pos=(175,60))

	def fill_speedpanel(self, speedpanel):	
		speedbox = wx.StaticBox(speedpanel, -1, 'Performance', (5,5), size=(220,200))

		wx.StaticText(speedpanel, -1, 'Speed:', pos=(20,30))
		self.speed = wx.TextCtrl(speedpanel, 41, 'DEFAULT', pos=(100,26), size=(60,-1))
		wx.StaticText(speedpanel, -1, 'knots', pos=(170,30))
		wx.StaticText(speedpanel, -1, 'desired HD:', pos=(20,60))
		self.des_heading = wx.TextCtrl(speedpanel, 42, 'DEFAULT', pos=(100,56), size=(60,-1))
		wx.StaticText(speedpanel, -1, 'degree', pos=(170,60))
		wx.StaticText(speedpanel, -1, 'current HD:', pos=(20,90))
		self.cur_heading = wx.TextCtrl(speedpanel, 43, 'DEFAULT', pos=(100,86), size=(60,-1))
		wx.StaticText(speedpanel, -1,'degree', pos=(170,90)) 

		wx.StaticLine(speedpanel, -1, (20,130),(180,1))

		wx.StaticText(speedpanel, -1, 'Drift:', pos=(20,140))
		self.drift = wx.TextCtrl(speedpanel, 48, 'DEFAULT', pos=(100,136), size=(60,-1))
		wx.StaticText(speedpanel, -1, 'knots', pos=(170,140))
		wx.StaticText(speedpanel, -1, 'Roll:', pos=(20,170))
		self.roll = wx.TextCtrl(speedpanel, 49, 'DEFAULT', pos=(100,166), size=(60,-1))
		wx.StaticText(speedpanel, -1,'degree', pos=(170,170)) 
	
	def fill_flagpanel(self, flagpanel):
		statbox = wx.StaticBox(flagpanel,-1, 'Boat Controller Information', (5,5), size=(390,160))

		self.joystick = wx.CheckBox(flagpanel, 35, 'Joystick', pos=(20,30))
		self.sailor = wx.CheckBox(flagpanel, 36, 'Sailor', pos=(100,30))
		self.autonomous = wx.CheckBox(flagpanel, 37, 'Fully Autonomous', pos=(180,30))
		self.txtsailingmode = wx.StaticText(flagpanel, -1, 'Sailing Mode:', pos=(20,70))
		#self.txtsailingmode.SetForegroundColour('white')
		self.sailMode = wx.TextCtrl(flagpanel, 40, 'DEFAULT', pos=(150,66), size=(200,-1))

		smallline = wx.StaticLine(flagpanel, -1,(20,100),(350,1))
		wx.StaticText(flagpanel, -1, 'RudderState Right:', pos=(20, 110))
		wx.StaticText(flagpanel, -1, 'RudderState Left:', pos=(20, 135))
		self.rudderright = wx.TextCtrl(flagpanel, 60, 'DFLT', pos=(150, 105), size=(80, -1))
		self.rudderleft = wx.TextCtrl(flagpanel, 61, 'DFLT', pos=(150, 130), size=(80, -1))
		wx.StaticText(flagpanel, -1, 'degree', pos=(260, 110))
		wx.StaticText(flagpanel, -1, 'degree', pos=(260, 135))

		#statbox.SetForegroundColour('white')
		#flagpanel.SetBackgroundColour('black')
		#self.joystick.SetForegroundColour('white')
		#self.sailor.SetForegroundColour('white')
		#self.autonomous.SetForegroundColour('white')
	
	def create_wyp_plot(self):
		self.wypfigure = Figure((6.0, 3.5), dpi =110, facecolor = 'w')

		self.wypfigure.subplots_adjust(top = 0.93, bottom = 0.04, left = 0.06, right = 0.95, wspace=None, hspace=None)
		self.wypplot = self.wypfigure.add_subplot(111) #, aspect = 'equal')

		pylab.setp(self.wypplot.get_xticklabels(), fontsize=6)
		pylab.setp(self.wypplot.get_yticklabels(), fontsize=6)
		self.wypplot.set_title('navigated path', size=8)

		self.plot_posdata = self.wypplot.plot(
				array(self.pos_longitude),
				array(self.pos_latitude),
				linewidth = 1,
				color = (1, 0, 0),
				label = 'AVALON',
				)[0]

		self.plot_wypdata = self.wypplot.plot(
				array(self.wyp_longitude),
				array(self.wyp_latitude),
				linewidth = 1,
				color = (0, 0, 1),
				label = 'calculated',
				)[0]

		myprop = matplotlib.font_manager.FontProperties(size=10)
		self.wypplot.legend(loc=4, shadow=True, prop=myprop)
		self.wypplot.set_axis_bgcolor('white')
		self.canvas = FigureCanvas(self, -1, self.wypfigure)


	def create_speed_plot(self):
		self.speed_panel = wx.Panel(self)
		self.dpi = 100
		self.fig = Figure((3.8, 1.95), dpi=self.dpi, facecolor = 'w')
		self.fig.subplots_adjust(top = 0.95,right = 0.97, bottom = 0.07 )

		self.speedplot = self.fig.add_subplot(111)
		self.speedplot.set_axis_bgcolor('black')
		#self.speedplot.set_title('AVALON Speed History', size=10)
		self.speedplot.set_ylabel('[knots]', size = 8)

		pylab.setp(self.speedplot.get_xticklabels(), fontsize=8)
		pylab.setp(self.speedplot.get_yticklabels(), fontsize=8)

		# plot the data as a line series, and save the reference 
		# to the plotted line series
		#
		self.plot_data = self.speedplot.plot(
				self.speeddata, 
				linewidth=1,
				color=(1, 1, 0),
				)[0]

        	self.speed_canvas = FigureCanvas(self.speed_panel, -1, self.fig)

	
	def UpdatePosPlot(self):
		# ---------------------------------------
		# redraws the position plot:
		# ---------------------------------------
		delta_long = max(max(self.pos_longitude),max(self.wyp_longitude)) - min(min(self.pos_longitude), min(self.wyp_longitude))
		long_m = 0.5*(max(max(self.pos_longitude), max(self.wyp_longitude)) + min(min(self.pos_longitude), min(self.wyp_longitude)))
		delta_lat = max(max(self.pos_latitude), max(self.wyp_latitude)) - min(min(self.pos_latitude), min(self.wyp_latitude))
		lat_m = 0.5*(max(max(self.pos_latitude), max(self.wyp_latitude)) + min(min(self.pos_latitude), min(self.wyp_latitude)))
		
		delta_long_calc = 6.0/3.5 * delta_lat
		delta_lat_calc = 3.5/6.0 * delta_long
		
		# --- set axes boundaries ---
		if delta_lat_calc > delta_lat:
			xmax = (long_m + 0.5 * delta_long + 0.0005)
			xmin = (long_m - 0.5 * delta_long - 0.0005)
			ymax = (lat_m + 0.5 * delta_lat_calc + 0.0005)
			ymin = (lat_m - 0.5 * delta_lat_calc - 0.0005)
		elif delta_long_calc > delta_long:
			xmax = (long_m + 0.5 * delta_long_calc + 0.0005)
			xmin = (long_m - 0.5 * delta_long_calc - 0.0005)
			ymax = (lat_m + 0.5 * delta_lat + 0.0005)
			ymin = (lat_m - 0.5 * delta_lat - 0.0005)
		else:
			# --- not 1:1 ratio ---
			xmax = max(self.pos_longitude) + 0.0005
			xmin = min(self.pos_longitude) - 0.0005
			ymax = max(self.pos_latitude) + 0.0005
			ymin = min(self.pos_latitude) - 0.0005

		#print long_m

		self.wypplot.set_xbound(lower=xmin, upper=xmax)
		self.wypplot.set_ybound(lower=ymin, upper=ymax)
		#self.wypplot.set_xbound(lower=-300, upper=300)
		#self.wypplot.set_ybound(lower=-200, upper=200)

		#grid on:
		self.wypplot.grid(True, color='gray')
		
		self.plot_posdata.set_xdata(array(self.pos_longitude))
		self.plot_posdata.set_ydata(array(self.pos_latitude))
		
		self.plot_wypdata.set_xdata(array(self.wyp_longitude))
		self.plot_wypdata.set_ydata(array(self.wyp_latitude))

		self.canvas.draw()

	def UpdateSpeedPlot(self):
		""" Redraws the plot
		"""
		# define the range of the axes 
		xmax = self.logtime[30] if len(self.logtime) > 30 else 60
		xmin = self.logtime[0]
		ymin = round(min(self.speeddata), 0) - 1
		ymax = round(max(self.speeddata), 0) + 1

		self.speedplot.set_xbound(lower=xmin, upper=xmax)
		self.speedplot.set_ybound(lower=ymin, upper=ymax)

		## grid on:
		self.speedplot.grid(True, color='gray')

		pylab.setp(self.speedplot.get_xticklabels(), 
		    visible=True)
		self.plot_data.set_xdata(array(self.logtime))
		self.plot_data.set_ydata(array(self.speeddata))

		self.speed_canvas.draw()

	def create_menu(self):
		menubar = wx.MenuBar()
		file = wx.Menu()
		file.Append(101, '&Open', 'not working momentarely')
		file.AppendSeparator()
		quit = wx.MenuItem(file, 102, '&Quit\tCtrl+Q','Quit the Application')
		file.AppendItem(quit)

		menudata = wx.Menu()
		menudata.Append(301, '&Initialize\tCtrl+I','Initialize Store-Data')
		menudata.Append(302, '&Start\tCtrl+S','Start reading Data')
		menudata.Append(303, '&Stop\tCtrl+E','Stop reading Data')
		menudata.Append(304, '&Plot WYP\tCtrl+W','fill wyps to vector')

		menuplot = wx.Menu()
		menuplot.Append(401, '&Clear Plot\tCtrl+C','Clear the Navigation plot')


		menubar.Append(file, '&File')
		menubar.Append(menudata,'&Data')
		menubar.Append(menuplot, '&Plot')
		self.SetMenuBar(menubar)
		self.Bind(wx.EVT_MENU, self.OnQuit, id=102)
		self.Bind(wx.EVT_MENU, self.OnInitData, id=301)
		self.Bind(wx.EVT_MENU, self.OnStartStop, id=302)
		self.Bind(wx.EVT_MENU, self.OnStartStop, id=303)
		self.Bind(wx.EVT_MENU, self.OnWypFill, id=304)
		self.Bind(wx.EVT_MENU, self.OnClearPlot, id=401)
		self.Centre()

	def OnClearPlot(self, event):
		# clear the data-arrays:
		del self.pos_longitude[:]
		del self.pos_latitude[:]
		del self.wyp_longitude[:]
		del self.wyp_latitude[:]

		msg = wx.MessageDialog(None, 'Plot data cleared!', 'Info', wx.OK)
		msg.ShowModal()

	def OnInitData(self, event):
		
		#-----------------------------------------------------------
		# initialize Store connection and define variables
		#-----------------------------------------------------------
		
		store = ddxInterface.ddxStore()
		self.IMUData = store.variable("imu")
		#self.FLAGData = store.variable("flags")
		#self.WINDData = store.variable("cleanwind")
		#self.DHData = store.variable("desiredheading")
		self.WypData = store.variable("wypData")
		self.DESTData = store.variable("destData")
		#self.RDRData = store.variable("rudderstateright")
		#self.RDLData = store.variable("rudderstateleft")

		#flag:
		self.initdone = 1

	def OnUpdate(self, event):
		###read DDX variables and write to GUI: ####
		self.IMUData.read()
		#self.FLAGData.read()
		#self.WINDData.read()
		#self.DHData.read()
		self.DESTData.read()
		#self.RDRData.read()
		#self.RDLData.read()

		###sailing mode: ####
		#mode = {
		#		1: 'IDLE',
		#		2: 'DOCK',
		#		3: 'NORMAL SAILING',
		#		4: 'TACK',
		#		5: 'JIBE',
		#		6: 'UPWIND',
		#		7: 'DOWNWIND',
		#		8: 'MAX ENERGY SAVING',
		#		9: 'HEADING CHANGE',
		#		0: 'ERROR',
		#		#}[8]
		#		}[int(self.FLAGData.state)]
		#
		#self.sailMode.SetValue(mode)
		####radio buttons: ####

		#joystick_bool = {1: False, 2: True, 0: False}[int(self.FLAGData.man_in_charge)]
		#sailor_bool = {1: True, 2: False, 0: False}[int(self.FLAGData.man_in_charge)]
		#autonomous_bool = {0: False, 1: True}[int(self.FLAGData.autonom_navigation)]
		##print self.FLAGData.man_in_charge
		##joystick_bool = {1: False, 2: True}[1]
		##sailor_bool = {1: True, 2: False}[1]
		##autonomous_bool = {0: False, 1: True}[1]
		#self.joystick.SetValue(joystick_bool)
		#self.sailor.SetValue(sailor_bool)
		#self.autonomous.SetValue(autonomous_bool)

		##self.rudderright.SetValue(' %1.3f') % (float(self.RDRData.degrees_rudder))
		#self.rudderright.SetValue(str(float(self.RDRData.degrees_rudder)))
		#self.rudderleft.SetValue(str(float(self.RDLData.degrees_rudder)))
		#
		##### performance information: ####
		##self.speed.SetValue(str(6.34))
		##self.des_heading.SetValue(str(130))
		##self.cur_heading.SetValue(str(120))
		##self.drift.SetValue(str(0.1))
		##self.roll.SetValue(str(16))
		#self.speed.SetValue(str(float(self.IMUData.speed)))
		#self.des_heading.SetValue(str(float(self.DHData.heading)))
		#self.cur_heading.SetValue(str(float(self.IMUData.attitude.yaw)))
		#self.drift.SetValue(str(float(self.IMUData.velocity.y)))
		#self.roll.SetValue(str(float(self.IMUData.attitude.roll)))

		##### environment information: ####
		#self.wind_speed.SetValue(str(float(self.WINDData.speed_long)))
		#self.wind_direction.SetValue(str(float(self.WINDData.global_direction_real_long)))


		##### energy panel information: ####
		#self.voltage.SetValue('--')
		
		# ------------------------------------------------------------------------------
		# plots:
		# ------------------------------------------------------------------------------
		#sp_length = len(self.logtime)
		#self.logtime.append((self.logtime[sp_length-1])+2)
		#self.speeddata.append(float(self.IMUData.speed))
		#if sp_length > 30:
		#	self.logtime.pop(0)
		#	self.speeddata.pop(0)

		#self.UpdateSpeedPlot()

		# ---- position decoding (every 10 seconds)------
		if self.plot_count > 1:
			self.plot_count = 0
			
			pos_x = float (EARTH_RAD * (math.pi/180)
				*(float(self.IMUData.position.latitude) - float(self.DESTData.latitude)))
			pos_y = float (EARTH_RAD 
				*math.cos((float(self.DESTData.latitude) * math.pi/180.0))*(math.pi/180.0)
				*(float(self.IMUData.position.longitude) - float(self.DESTData.longitude)))

			self.pos_longitude.append(pos_y) #(float(self.IMUData.position.longitude))
			self.pos_latitude.append(pos_x) #(float(self.IMUData.position.latitude)) 
			print self.pos_longitude
			print self.pos_latitude

			#if len(self.pos_longitude) > 60:
			#	self.pos_longitude.pop(0)
			#	self.pos_latitude.pop(0)


			self.UpdatePosPlot()

			
		self.plot_count += 1	
		#print self.plot_count
		


		###debug:
		print 'updated control panel'

	def OnWypFill(self, event):

		self.WypData.read()

		del self.wyp_longitude[:]
		del self.wyp_latitude[:]

		for i in range(0, 18):
			self.wyp_longitude.append(float(self.WypData.Data[i].y))
			self.wyp_latitude.append(float(self.WypData.Data[i].x))

			print self.wyp_longitude
			print self.wyp_latitude
			if int(self.WypData.Data[i].wyp_type) == 1:
				# --- stop ---
				break

		self.UpdatePosPlot()
		print self.wyp_longitude
		print 'Wyp Data filled'



	def OnStartStop(self, event):
		if self.initdone == 0:
			msg = wx.MessageDialog(None, 'Store not initialized. Please press Ctrl-I', 'Error', wx.OK | 
            			wx.ICON_EXCLAMATION)
        		msg.ShowModal()
			return 

		if self.timer.IsRunning():
			self.timer.Stop()
			#self.startBtn.SetLabel("Restart")
			#self.fileHandle.close()
			print "timer stopped"
		else:
			print "starting timer"
			self.timer.Start(2000)
			#os.remove('position.txt')
			#self.fileHandle = open('position.txt','a')
			#self.startBtn.SetLabel("Stop")

	def OnQuit(self, event):
		self.Destroy()

##### heading indicator: #######
class HeadingFrame(wx.Frame):
        def __init__(self, parent, id, title):
        	wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(640,800))

		##### Create Menu: #####
		menubar = wx.MenuBar()
		file = wx.Menu()
		file.Append(201, '&Initialize\tCtrl+I', 'initialize frame')
		file.Append(203, '&Stop update\tCtrl+S', 'stop reading DDX data')
		file.AppendSeparator()
		quit = wx.MenuItem(file, 202, '&Quit\tCtrl+Q','Quit the Application')
		file.AppendItem(quit)

		menubar.Append(file, '&File')
		self.SetMenuBar(menubar)
		self.Bind(wx.EVT_MENU, self.onQuit, id=202)
		self.Bind(wx.EVT_MENU, self.onInit, id=201)
		self.Bind(wx.EVT_MENU, self.onStop, id=203)
		self.Centre()

		#### setup timer: ####
		self.timer = wx.Timer(self)
        	self.Bind(wx.EVT_TIMER, self.OnUpdate, self.timer)

		#### setup picture panel: ####
       		#sizer = wx.BoxSizer(wx.VERTICAL)	
		panel = wx.Panel(self,-1,size=(600,600), style=wx.SUNKEN_BORDER)
		self.comp = wx.StaticBitmap(panel)
		#self.comp.SetPosition((89.5,89.5))
		#self.comp.Center()
		self.boat = wx.StaticBitmap(panel)
		self.wind = wx.StaticBitmap(panel)
		#self.tacho = wx.StaticBitmap(panel)
		#self.boat.SetPosition((100,100))
		panel.SetBackgroundColour('black')
		panel.SetPosition((20,20))
		#panel.Center()
        	wx.EVT_PAINT(self, self.OnPaint)
		self.SetBackgroundColour('black')


		#### on/off:
		self.status = wx.StaticText(panel, -1, 'OFF', pos=(550,570))
		self.status.SetForegroundColour('white')

		#### text below: ####
		self.text = wx.StaticText(self, -1, 'Wind speed:', pos=(30,645))
		self.text2 = wx.StaticText(self, -1, 'knots', pos=(280, 645))
		self.text.SetForegroundColour('white')
		self.text2.SetForegroundColour('white')
		newfont = wx.Font(12, wx.TELETYPE, wx.NORMAL, wx.NORMAL)
		self.text.SetFont(newfont)
		self.text2.SetFont(newfont)
		self.wspeed = gizmos.LEDNumberCtrl(self, 100, wx.DefaultPosition, wx.DefaultSize, gizmos.LED_ALIGN_RIGHT)
		self.wspeed.SetPosition((180, 635))
		self.wspeed.SetSize((90,30))

		#### text below: ####
		self.text3 = wx.StaticText(self, -1, 'Wind direction:', pos=(30,685))
		self.text4 = wx.StaticText(self, -1, 'degree', pos=(280, 685))
		self.text3.SetForegroundColour('white')
		self.text4.SetForegroundColour('white')
		self.text3.SetFont(newfont)
		self.text4.SetFont(newfont)
		self.wdirect = gizmos.LEDNumberCtrl(self, 100) #, gizmos.LED_ALIGN_CENTER)
		self.wdirect.SetPosition((180, 675))
		self.wdirect.SetSize((90,30))

		#pen = wx.Pen('#4c4c4c',5,wx.SOLID)

		#self.dc = wx.ClientDC(self)
		#self.dc.SetPen(pen)
		#self.dc.DrawLine(50, 700, 160, 700)


    	def OnPaint(self, event):
        	compass = wx.Image('pics/hd_comp2.png')
		wind = wx.Image('pics/hd_wind2.png')
		boat = wx.Bitmap('pics/hd_boat1.png')
		#tacho = wx.Bitmap('pics/tacho1.png')

		compassrot = compass.ConvertToBitmap()
		windrot = wind.ConvertToBitmap()
		self.comp.SetBitmap(compassrot)
		self.boat.SetBitmap(boat)
		self.wind.SetBitmap(windrot)
		#self.tacho.SetBitmap(tacho)
		self.comp.SetPosition((89.5,89.5))
		self.wind.SetPosition((89.5,89.5))
		self.boat.SetPosition((89.5,89.5))


		#dc = wx.PaintDC(self)
       		#dc.SetPen(wx.Pen('#d4d4d4'))
		#dc.SetBrush(wx.Brush('#4c4c4c'))
	        #dc.DrawRectangle(130, 300, 90, 60)
		self.firsttime = 1


	def onInit(self,event):
		if not self.timer.IsRunning():
			self.timer.Start(2000)
			self.status.SetLabel('ON')
			print 'head indicator reading initialized'

	def onStop(self, event):
		if self.timer.IsRunning():
			self.timer.Stop()
			self.status.SetLabel('OFF')
			print 'head indicator reading stopped'

	def OnUpdate(self, event):
		print 'updating heading indicator'
		##### reading from store:
		#IMUData.read()
		#WINDData.read()
		
		#compassrad = float(IMUData.attitude.yaw)
		#windrad = -float(WINDData.global_direction_real_long) + compassrad
		compassrad = float(120)
		windrad = -float(-30) + compassrad

		compassrad = math.radians(compassrad)
		windrad = math.radians(windrad)

		comp_offset = 300 - math.sin(math.radians(45) + math.fabs(math.fmod(compassrad,math.radians(90))))*297.69195
		wind_offset = 300 - math.sin(math.radians(45) + math.fabs(math.fmod(windrad,math.radians(90))))*297.69195

		##### text updates:
		#speed ='%.1f' % 13.6
		#direct = '%.1f' % 120.234
		#self.wspeed.SetValue(str(float(WINDData.speed_long)))
		#self.wdirect.SetValue(str(float(WINDData.global_direction_real_long)))
		self.wspeed.SetValue(str((25.789)))
		self.wdirect.SetValue(str((-30)))

		##### image updates:
        	compass = wx.Image('pics/hd_comp2.png')
		wind = wx.Image('pics/hd_wind2.png')
		#boat = wx.Bitmap('pics/hd_boat1.png')

		compass = compass.Rotate(compassrad,(210.5,210.5))
		wind = wind.Rotate(windrad,(210.5,210.5))
		compassrot = compass.ConvertToBitmap()
		windrot = wind.ConvertToBitmap()

		self.comp.SetBitmap(compassrot)
		self.comp.SetPosition((comp_offset,comp_offset))
		#self.boat.SetBitmap(boat)
		self.wind.SetBitmap(windrot)
		self.wind.SetPosition((wind_offset, wind_offset))

	def onQuit(self,event):
		self.Destroy()


class MyApp(wx.App):
    def OnInit(self):
        frame1 = ControlFrame(None, -1,'AVALON COCKPIT')
        frame1.Show(True)
	#frame2 = HeadingFrame(None, -1, 'HEADING INDICATOR')
	#frame2.Show(True)
	#frame3 = PlotFrame(None, -1, 'PLOT CONTROL')
	#frame3.Show(True)
        return True
  
app = MyApp(0)
app.MainLoop()
