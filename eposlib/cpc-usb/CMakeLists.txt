SET(CPC_USB_SOURCES cpclib.c cpc_async.c ethercan.c ini_parser.c)
ADD_DEFINITIONS(-DCPC_LIB_VERSION="2.42")

ADD_LIBRARY(
	cpc
	SHARED
	${CPC_USB_SOURCES}
)

INSTALL_TARGETS(/lib cpc)
