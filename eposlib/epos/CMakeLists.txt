ADD_LIBRARY(
	epos
	SHARED
	epos.c
)

INSTALL_TARGETS(/lib/ epos)
INSTALL_FILES(/include/can
	.h
	epos
)


