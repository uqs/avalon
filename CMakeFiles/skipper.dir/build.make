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
include CMakeFiles/skipper.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/skipper.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/skipper.dir/flags.make

CMakeFiles/skipper.dir/skipper.o: CMakeFiles/skipper.dir/flags.make
CMakeFiles/skipper.dir/skipper.o: skipper.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/stefan/RoboYacht/svn/avalon/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/skipper.dir/skipper.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/skipper.dir/skipper.o -c /home/stefan/RoboYacht/svn/avalon/skipper.cpp

CMakeFiles/skipper.dir/skipper.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/skipper.dir/skipper.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/stefan/RoboYacht/svn/avalon/skipper.cpp > CMakeFiles/skipper.dir/skipper.i

CMakeFiles/skipper.dir/skipper.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/skipper.dir/skipper.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/stefan/RoboYacht/svn/avalon/skipper.cpp -o CMakeFiles/skipper.dir/skipper.s

CMakeFiles/skipper.dir/skipper.o.requires:
.PHONY : CMakeFiles/skipper.dir/skipper.o.requires

CMakeFiles/skipper.dir/skipper.o.provides: CMakeFiles/skipper.dir/skipper.o.requires
	$(MAKE) -f CMakeFiles/skipper.dir/build.make CMakeFiles/skipper.dir/skipper.o.provides.build
.PHONY : CMakeFiles/skipper.dir/skipper.o.provides

CMakeFiles/skipper.dir/skipper.o.provides.build: CMakeFiles/skipper.dir/skipper.o
.PHONY : CMakeFiles/skipper.dir/skipper.o.provides.build

# Object files for target skipper
skipper_OBJECTS = \
"CMakeFiles/skipper.dir/skipper.o"

# External object files for target skipper
skipper_EXTERNAL_OBJECTS =

skipper: CMakeFiles/skipper.dir/skipper.o
skipper: CMakeFiles/skipper.dir/build.make
skipper: CMakeFiles/skipper.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable skipper"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/skipper.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/skipper.dir/build: skipper
.PHONY : CMakeFiles/skipper.dir/build

CMakeFiles/skipper.dir/requires: CMakeFiles/skipper.dir/skipper.o.requires
.PHONY : CMakeFiles/skipper.dir/requires

CMakeFiles/skipper.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/skipper.dir/cmake_clean.cmake
.PHONY : CMakeFiles/skipper.dir/clean

CMakeFiles/skipper.dir/depend:
	cd /home/stefan/RoboYacht/svn/avalon && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon/CMakeFiles/skipper.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/skipper.dir/depend

