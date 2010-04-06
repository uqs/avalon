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
import wx
#import time	
import sys
sys.path.append('/usr/local/lib/python2.6/site-packages/')
import ddxInterface
import wx.gizmos as gizmos
import math
 
#### initialize store connection and create store variables: ####
#store = ddxInterface.ddxStore()
#IMUData = store.variable("imu")
#FLAGData = store.variable("flags")
#WINDData = store.variable("cleanwind")
#DHData = store.variable("desiredheading")

#### main frame: ####
class ControlFrame(wx.Frame):
        def __init__(self, parent, id, title):
        	wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(500, 800))
		
		##### Create Menu: #####
		menubar = wx.MenuBar()
		file = wx.Menu()
		file.Append(101, '&Open', 'not working momentarely')
		file.AppendSeparator()
		quit = wx.MenuItem(file, 102, '&Quit\tCtrl+Q','Quit the Application')
		file.AppendItem(quit)

		menubar.Append(file, '&File')
		self.SetMenuBar(menubar)
		self.Bind(wx.EVT_MENU, self.OnQuit, id=102)
		self.Centre()
		
		#### set main sizers: ####	
		mainbox = wx.BoxSizer(wx.VERTICAL)
		perfbox = wx.BoxSizer(wx.HORIZONTAL)
		rightbox = wx.BoxSizer(wx.VERTICAL)

		#### panels: ####
		#ctrlPanel = wx.Panel(self, -1, size=(550,-1))
		boxpanel = wx.Panel(self, -1, size=(500,150))
		envpanel = wx.Panel(self, -1, size=(230,150))
		speedpanel = wx.Panel(self, -1, size=(230,150))
		energypanel = wx.Panel(self, -1, size=(230,150))
		#envpanel.SetBackgroundColour('white')
		#speedpanel.SetBackgroundColour('red')
		#boxpanel.SetBackgroundColour('green')
		#energypanel.SetBackgroundColour('yellow')

		#### setup timer: ####
		self.timer = wx.Timer(self)
        	self.Bind(wx.EVT_TIMER, self.onUpdate, self.timer)
		

		fatfont = wx.Font(10, wx.DEFAULT, wx.NORMAL, wx.BOLD)
		title = wx.StaticText(self, 1, 'Control Panel')
		title.SetFont(fatfont)
		line = wx.StaticLine(self, -1,(10,10),(460,1))
		line2 = wx.StaticLine(self, -1,(10,10),(460,1))
		line3 = wx.StaticLine(self, -1,(10,10),(460,1))
		
		#mainbox.Add(ctrlPanel, 1, wx.EXPAND, 0)
		mainbox.Add((0,20),0)
		mainbox.Add(title, 0, wx.ALIGN_CENTER, 20)
		#mainbox.Add((0,4),0)
		mainbox.Add(line,0,wx.ALIGN_CENTER | wx.ALL ,10)
		#mainbox.Add((0,4),0)
		
		#### Control Buttons: ####
		ctrl_sizer = wx.GridSizer(2,4,3,3)
		self.startBtn = wx.Button(self, 10, 'Initialize')
		self.idleBtn = wx.Button(self, 14, 'MAX E SAVING')
		self.joystickBtn = wx.Button(self, 15, 'JOYSTICK')
		self.sailorBtn = wx.Button(self, 16, 'SAILOR')
		self.fullBtn = wx.Button(self, 17,'AUTONOMOUS')
		self.startBtn.SetForegroundColour('black')
		self.idleBtn.SetForegroundColour('black')
		self.joystickBtn.SetForegroundColour('black')
		self.sailorBtn.SetForegroundColour('black')
		self.fullBtn.SetForegroundColour('black')
		
		ctrl_sizer.AddMany([(self.startBtn,0,wx.EXPAND),
			(wx.Button(self,11,'Carribean Seas'),0,wx.EXPAND),
			(wx.StaticText(self, -1, ''),0,wx.EXPAND),
			(wx.Button(self,12,'Self Destroy'),0,wx.EXPAND),
			(self.idleBtn, 0, wx.EXPAND),
			(self.joystickBtn, 0, wx.EXPAND),
			(self.sailorBtn, 0, wx.EXPAND),
			(self.fullBtn, 0, wx.EXPAND) ])

		mainbox.Add(ctrl_sizer, 0, wx.ALIGN_CENTER | wx.ALL, 5)
		mainbox.Add(line2,0,wx.ALIGN_CENTER | wx.ALL ,20)

		self.startBtn.Bind(wx.EVT_BUTTON, self.onInit)
		
		### controller info: ####
		statbox = wx.StaticBox(boxpanel,-1, 'Boat Controller Information', (5,5), size=(450,140))
		self.joystick = wx.CheckBox(boxpanel, 35, 'Joystick', pos=(20,30))
		self.sailor = wx.CheckBox(boxpanel, 36, 'Sailor', pos=(100,30))
		self.autonomous = wx.CheckBox(boxpanel, 37, 'Fully Autonomous', pos=(180,30))
		wx.StaticText(boxpanel, -1, 'Sailing Mode:', pos=(20,70))
		self.sailMode = wx.TextCtrl(boxpanel, 40, 'DEFAULT', pos=(150,66), size=(200,-1))

		mainbox.Add(boxpanel,0,wx.ALIGN_CENTER | wx.RIGHT | wx.LEFT, 15)
		mainbox.Add(line3,0,wx.ALIGN_CENTER | wx.ALL ,5)

		#### speed and energy box: ####
		speedbox = wx.StaticBox(speedpanel, -1, 'Performance', (5,5), size=(220,300))
		envbox = wx.StaticBox(envpanel, -1, 'Environment', (5,5), size=(220,140))
		energybox = wx.StaticBox(energypanel, -1, 'Energy Management', (5,5), size=(220,140))
	
		wx.StaticText(speedpanel, -1, 'Speed:', pos=(20,30))
		self.speed = wx.TextCtrl(speedpanel, 41, 'DEFAULT', pos=(100,26), size=(60,-1))
		wx.StaticText(speedpanel, -1, 'knots', pos=(170,30))
		wx.StaticText(speedpanel, -1, 'desired HD:', pos=(20,60))
		self.des_heading = wx.TextCtrl(speedpanel, 42, 'DEFAULT', pos=(100,56), size=(60,-1))
		wx.StaticText(speedpanel, -1, 'degrees', pos=(170,60))
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
		perfbox.Add(speedpanel)

		wx.StaticText(envpanel, -1, 'Wind Speed:', pos=(20,30))
		self.wind_speed = wx.TextCtrl(envpanel, 44, 'DEFAULT', pos=(110,26), size=(60,-1))
		wx.StaticText(envpanel, -1, 'knots', pos=(175,30))
		wx.StaticText(envpanel, -1, 'Direction:', pos=(20,60))
		self.wind_direction = wx.TextCtrl(envpanel, 45, 'DEFAULT', pos=(110,56), size=(60,-1))
		wx.StaticText(envpanel, -1, 'degree', pos=(175,60))
		
		wx.StaticText(energypanel, -1, 'Voltage:', pos=(20,30))
		self.voltage = wx.TextCtrl(energypanel, 46, 'DEFAULT', pos=(110,26), size=(60,-1))
		wx.StaticText(energypanel, -1, 'Volts', pos=(175,30))

		rightbox.Add(envpanel)
		rightbox.Add(energypanel)

		perfbox.Add(rightbox, 0, wx.ALIGN_CENTER | wx.RIGHT, 0)
		mainbox.Add(perfbox, 0, wx.ALIGN_CENTER | wx.ALL , 20)
	
		self.SetSizer(mainbox)

	def onUpdate(self, event):
		###read DDX variables and write to GUI: ####
		IMUData.read()
		FLAGData.read()
		WINDData.read()
		DHData.read()

		###sailing mode: ####
		mode = {
				1: 'IDLE',
				2: 'DOCK',
				3: 'NORMAL SAILING',
				4: 'TACK',
				5: 'JIBE',
				6: 'UPWIND',
				7: 'DOWNWIND',
				8: 'MAX ENERGY SAVING',
				9: 'HEADING CHANGE',
				0: 'ERROR',
				#}[8]
				}[int(FLAGData.state)]
		
		self.sailMode.SetValue(mode)
		###radio buttons: ####

		joystick_bool = {1: False, 2: True, 0: False}[int(FLAGData.man_in_charge)]
		sailor_bool = {1: True, 2: False, 0: False}[int(FLAGData.man_in_charge)]
		autonomous_bool = {0: False, 1: True}[int(FLAGData.autonom_navigation)]
		#joystick_bool = {1: False, 2: True}[1]
		#sailor_bool = {1: True, 2: False}[1]
		#autonomous_bool = {0: False, 1: True}[1]
		self.joystick.SetValue(joystick_bool)
		self.sailor.SetValue(sailor_bool)
		self.autonomous.SetValue(autonomous_bool)
	
		#### performance information: ####
		self.speed.SetValue(str(float(IMUData.speed)))
		self.des_heading.SetValue(str(float(DHData.heading)))
		self.cur_heading.SetValue(str(float(IMUData.attitude.yaw)))
		self.drift.SetValue(str(float(IMUData.velocity.y)))
		self.roll.SetValue(str(float(IMUData.attitude.roll)))

		#### environment information: ####
		self.wind_speed.SetValue(str(float(WINDData.speed_long)))
		self.wind_direction.SetValue(str(float(WINDData.global_direction_real_long)))


		#### energy panel information: ####
		self.voltage.SetValue('--')
		
		###debug:
		print 'updating control panel'


	def onInit(self, event):
		if self.timer.IsRunning():
			self.timer.Stop()
			self.startBtn.SetLabel("Restart")
			print "timer stopped"
		else:
			print "starting timer"
			self.timer.Start(2000)
			self.startBtn.SetLabel("Stop")

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
        	self.Bind(wx.EVT_TIMER, self.onUpdate, self.timer)

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

	def onUpdate(self, event):
		print 'updating heading indicator'
		##### reading from store:
		IMUData.read()
		WINDData.read()
		
		compassrad = float(IMUData.attitude.yaw)
		windrad = -float(WINDData.global_direction_real_long) + compassrad
		#compassrad = float(120)
		#windrad = -float(-30) + compassrad

		compassrad = math.radians(compassrad)
		windrad = math.radians(windrad)

		comp_offset = 300 - math.sin(math.radians(45) + math.fabs(math.fmod(compassrad,math.radians(90))))*297.69195
		wind_offset = 300 - math.sin(math.radians(45) + math.fabs(math.fmod(windrad,math.radians(90))))*297.69195

		##### text updates:
		#speed ='%.1f' % 13.6
		#direct = '%.1f' % 120.234
		self.wspeed.SetValue(str(float(WINDData.speed_long)))
		self.wdirect.SetValue(str(float(WINDData.global_direction_real_long)))

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
        #frame1 = ControlFrame(None, -1,'AVALON COCKPIT')
        #frame1.Show(True)
	frame2 = HeadingFrame(None, -1, 'HEADING INDICATOR')
	frame2.Show(True)
        return True
  
app = MyApp(0)
app.MainLoop()
