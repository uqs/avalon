# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.6

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/stefan/RoboYacht/svn/avalon

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/stefan/RoboYacht/svn/avalon

# Include any dependencies generated for this target.
include CMakeFiles/testaufruf.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/testaufruf.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testaufruf.dir/flags.make

CMakeFiles/testaufruf.dir/testaufruf.o: CMakeFiles/testaufruf.dir/flags.make
CMakeFiles/testaufruf.dir/testaufruf.o: testaufruf.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/stefan/RoboYacht/svn/avalon/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/testaufruf.dir/testaufruf.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/testaufruf.dir/testaufruf.o -c /home/stefan/RoboYacht/svn/avalon/testaufruf.cpp

CMakeFiles/testaufruf.dir/testaufruf.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testaufruf.dir/testaufruf.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/stefan/RoboYacht/svn/avalon/testaufruf.cpp > CMakeFiles/testaufruf.dir/testaufruf.i

CMakeFiles/testaufruf.dir/testaufruf.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testaufruf.dir/testaufruf.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/stefan/RoboYacht/svn/avalon/testaufruf.cpp -o CMakeFiles/testaufruf.dir/testaufruf.s

CMakeFiles/testaufruf.dir/testaufruf.o.requires:
.PHONY : CMakeFiles/testaufruf.dir/testaufruf.o.requires

CMakeFiles/testaufruf.dir/testaufruf.o.provides: CMakeFiles/testaufruf.dir/testaufruf.o.requires
	$(MAKE) -f CMakeFiles/testaufruf.dir/build.make CMakeFiles/testaufruf.dir/testaufruf.o.provides.build
.PHONY : CMakeFiles/testaufruf.dir/testaufruf.o.provides

CMakeFiles/testaufruf.dir/testaufruf.o.provides.build: CMakeFiles/testaufruf.dir/testaufruf.o
.PHONY : CMakeFiles/testaufruf.dir/testaufruf.o.provides.build

# Object files for target testaufruf
testaufruf_OBJECTS = \
"CMakeFiles/testaufruf.dir/testaufruf.o"

# External object files for target testaufruf
testaufruf_EXTERNAL_OBJECTS =

testaufruf: CMakeFiles/testaufruf.dir/testaufruf.o
testaufruf: CMakeFiles/testaufruf.dir/build.make
testaufruf: CMakeFiles/testaufruf.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable testaufruf"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testaufruf.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testaufruf.dir/build: testaufruf
.PHONY : CMakeFiles/testaufruf.dir/build

CMakeFiles/testaufruf.dir/requires: CMakeFiles/testaufruf.dir/testaufruf.o.requires
.PHONY : CMakeFiles/testaufruf.dir/requires

CMakeFiles/testaufruf.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testaufruf.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testaufruf.dir/clean

CMakeFiles/testaufruf.dir/depend:
	cd /home/stefan/RoboYacht/svn/avalon && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon/CMakeFiles/testaufruf.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testaufruf.dir/depend

