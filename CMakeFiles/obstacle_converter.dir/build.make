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
include CMakeFiles/obstacle_converter.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/obstacle_converter.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/obstacle_converter.dir/flags.make

CMakeFiles/obstacle_converter.dir/obstacle_converter.o: CMakeFiles/obstacle_converter.dir/flags.make
CMakeFiles/obstacle_converter.dir/obstacle_converter.o: obstacle_converter.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/stefan/RoboYacht/svn/avalon/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/obstacle_converter.dir/obstacle_converter.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/obstacle_converter.dir/obstacle_converter.o -c /home/stefan/RoboYacht/svn/avalon/obstacle_converter.cpp

CMakeFiles/obstacle_converter.dir/obstacle_converter.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/obstacle_converter.dir/obstacle_converter.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/stefan/RoboYacht/svn/avalon/obstacle_converter.cpp > CMakeFiles/obstacle_converter.dir/obstacle_converter.i

CMakeFiles/obstacle_converter.dir/obstacle_converter.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/obstacle_converter.dir/obstacle_converter.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/stefan/RoboYacht/svn/avalon/obstacle_converter.cpp -o CMakeFiles/obstacle_converter.dir/obstacle_converter.s

CMakeFiles/obstacle_converter.dir/obstacle_converter.o.requires:
.PHONY : CMakeFiles/obstacle_converter.dir/obstacle_converter.o.requires

CMakeFiles/obstacle_converter.dir/obstacle_converter.o.provides: CMakeFiles/obstacle_converter.dir/obstacle_converter.o.requires
	$(MAKE) -f CMakeFiles/obstacle_converter.dir/build.make CMakeFiles/obstacle_converter.dir/obstacle_converter.o.provides.build
.PHONY : CMakeFiles/obstacle_converter.dir/obstacle_converter.o.provides

CMakeFiles/obstacle_converter.dir/obstacle_converter.o.provides.build: CMakeFiles/obstacle_converter.dir/obstacle_converter.o
.PHONY : CMakeFiles/obstacle_converter.dir/obstacle_converter.o.provides.build

# Object files for target obstacle_converter
obstacle_converter_OBJECTS = \
"CMakeFiles/obstacle_converter.dir/obstacle_converter.o"

# External object files for target obstacle_converter
obstacle_converter_EXTERNAL_OBJECTS =

obstacle_converter: CMakeFiles/obstacle_converter.dir/obstacle_converter.o
obstacle_converter: CMakeFiles/obstacle_converter.dir/build.make
obstacle_converter: CMakeFiles/obstacle_converter.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable obstacle_converter"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/obstacle_converter.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/obstacle_converter.dir/build: obstacle_converter
.PHONY : CMakeFiles/obstacle_converter.dir/build

CMakeFiles/obstacle_converter.dir/requires: CMakeFiles/obstacle_converter.dir/obstacle_converter.o.requires
.PHONY : CMakeFiles/obstacle_converter.dir/requires

CMakeFiles/obstacle_converter.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/obstacle_converter.dir/cmake_clean.cmake
.PHONY : CMakeFiles/obstacle_converter.dir/clean

CMakeFiles/obstacle_converter.dir/depend:
	cd /home/stefan/RoboYacht/svn/avalon && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon /home/stefan/RoboYacht/svn/avalon/CMakeFiles/obstacle_converter.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/obstacle_converter.dir/depend
