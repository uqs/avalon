# Indicate this is not a GNU project and that
# missing files should be automatically created
AUTOMAKE_OPTIONS=foreign

# Global C Flags
#AM_CFLAGS = -pthread -g
AM_CPPFLAGS = -g -pthread -Wall -I.
AM_LDFLAGS = -pthread

# configuration library
SUBDIRS = configlib

# Programs to build
bin_PROGRAMS = MessageManagerNotifier MessageManager DataLogger WatchDog Tester PowerManager

MessageManagerNotifier_SOURCES = notifier.cpp PPPHandler.cpp Curl.cpp SMS.cpp Serial.cpp TransportHandler.cpp Exception.cpp
MessageManagerNotifier_LDFLAGS = @LIBCURL_LIBS@ -pthread
MessageManagerNotifier_CFLAGS = @LIBCURL_CPPFLAGS@

MessageManager_SOURCES = mm.cpp utils.cpp MessageServer.cpp MessageManager.cpp Logstream.cpp Thread.cpp Log.cpp PPPHandler.cpp EthernetHandler.cpp SMS.cpp IridiumHandler.cpp Serial.cpp TransportHandler.cpp GPRSHandler.cpp Exception.cpp GPIO.cpp PowerManager.cpp SoftwareWatchdog.cpp Curl.cpp
MessageManager_LDADD = configlib/libconfiglib.a
MessageManager_LDFLAGS = @LIBCURL_LIBS@ -pthread
MessageManager_CFLAGS = @LIBCURL_CPPFLAGS@

DataLogger_SOURCES = dl.cpp utils.cpp DataLogger.cpp DataSource.cpp GPSSource.cpp ElectricalSource.cpp Exception.cpp GPIO.cpp Log.cpp Logstream.cpp Thread.cpp SoftwareWatchdog.cpp EnvironmentSource.cpp PictureSource.cpp VideoSource.cpp PowerManager.cpp
DataLogger_LDADD = configlib/libconfiglib.a
DataLogger_LDFLAGS = -pthread
DataLogger_CFLAGS =

WatchDog_SOURCES = wd.cpp utils.cpp GPIO.cpp Logstream.cpp Exception.cpp SoftwareWatchdog.cpp
WatchDog_LDADD = configlib/libconfiglib.a

Tester_SOURCES = test/Tester.cpp test/SMSTester.cpp SMS.cpp Serial.cpp test/SerialTester.cpp test/MessageServerTester.cpp test/MessageManagerTester.cpp MessageManager.cpp MessageServer.cpp Logstream.cpp Thread.cpp Log.cpp test/EthernetTester.cpp EthernetHandler.cpp PPPHandler.cpp TransportHandler.cpp GPRSHandler.cpp test/PPPHandlerTester.cpp IridiumHandler.cpp Exception.cpp GPSSource.cpp ElectricalSource.cpp GPIO.cpp test/GPIOTester.cpp DataSource.cpp test/DataSourceTester.cpp PowerManager.cpp test/SharedMemoryTester.cpp SoftwareWatchdog.cpp EnvironmentSource.cpp Curl.cpp test/CurlTester.cpp PictureSource.cpp VideoSource.cpp
Tester_LDADD = configlib/libconfiglib.a
Tester_LDFLAGS = @LIBCURL_LIBS@ -pthread
Tester_CFLAGS = @LIBCURL_CPPFLAGS@

PowerManager_SOURCES = pm.cpp utils.cpp PowerManager.cpp Logstream.cpp Exception.cpp
PowerManager_LDADD = configlib/libconfiglib.a

