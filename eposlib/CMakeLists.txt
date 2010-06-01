cmake_minimum_required(VERSION 2.6)

SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/avalon")


ADD_DEFINITIONS( -ggdb -Wall )


INCLUDE_DIRECTORIES(
	${CMAKE_HOME_DIRECTORY}
    /usr/local/include
)

LINK_DIRECTORIES(
)


# SUBDIRS(can can-serial epos)
SUBDIRS(can can-serial epos example)




