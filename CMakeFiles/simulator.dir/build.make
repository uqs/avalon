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
include CMakeFiles/simulator.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/simulator.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/simulator.dir/flags.make

CMakeFiles/simulator.dir/simulator.o: CMakeFiles/simulator.dir/flags.make
CMakeFiles/simulator.dir/simulator.o: simulator.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/stefan/RoboYacht/svn/avalon/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/simulator.dir/simulator.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/simulator.dir/simulator.o -c /home/stefan/RoboYacht/svn/avalon/simulator.cpp

CMakeFiles/simulator.dir/simulator.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simulator.dir/simulator.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/stefan/RoboYacht/svn/avalon/simulator.cpp > CMakeFiles/simulator.dir/simulator.i

CMakeFiles/simulator.dir/simulator.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simulator.dir/simulator.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/stefan/RoboYacht/svn/avalon/simulator.cpp -o CMakeFiles/simulator.dir/simulator.s

CMakeFiles/simulator.dir/simulator.o.requires:
.PHONY : CMakeFiles/simulator.dir/simulator.o.requires

CMakeFiles/simulator.dir/simulator.o.provides: CMakeFiles/simulator.dir/simulator.o.requires
	$(MAKE) -f CMakeFiles/simulator.dir/build.make CMakeFiles/simulator.dir/simulator.o.provides.build
.PHONY : CMakeFiles/simulator.dir/simulator.o.provides

CMakeFiles/simulator.dir/simulator.o.provides.build: CMakeFiles/simulator.dir/simulator.o
.PHONY : CMakeFiles/simulator.dir/simulator.o.provides.build

# Object files for target simulator
simulator_OBJECTS = \
"CMakeFiles/simulator.dir/simulator.o"

# External object files for target simulator
simulator_EXTERNAL_OBJECTS =

simulator: CMakeFiles/simulator.dir/simulator.o
simulator: CMakeFiles/simulator.dir/build.make
simulator: CMakeFiles/simulator.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable simulator"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simulator.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/simulator.dir/build: simulator
.PHONY : CMakeFiles/simulator.dir/build

CMakeFiles/simulator.dir/requires: CMakeFiles/simulator.dir/simulator.o.requires
.PHONY : CMakeFiles/simulator.dir/requires

CMakeFiles/simulator.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simulator.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simulator.dir/clean

CMakeFiles/simulator.dir/depend:
	cd /home/stefan/RoboYacht/svn/avalon && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon/CMakeFiles/simulator.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simulator.dir/depend

