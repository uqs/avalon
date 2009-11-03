ADD_LIBRARY(
	can-serial
	SHARED
	can_serial.c
)

INSTALL_TARGETS(/lib/ can-serial)
INSTALL_FILES(/include/can
	.h
	can_serial
)


