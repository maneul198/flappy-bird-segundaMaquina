# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.3

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
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

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build

# Include any dependencies generated for this target.
include CMakeFiles/enginesv.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/enginesv.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/enginesv.dir/flags.make

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o: CMakeFiles/enginesv.dir/flags.make
CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o: /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv/enginesv_dpx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv/enginesv_dpx.cpp

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/enginesv.dir/enginesv_dpx.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv/enginesv_dpx.cpp > CMakeFiles/enginesv.dir/enginesv_dpx.cpp.i

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/enginesv.dir/enginesv_dpx.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv/enginesv_dpx.cpp -o CMakeFiles/enginesv.dir/enginesv_dpx.cpp.s

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.requires:

.PHONY : CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.requires

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.provides: CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.requires
	$(MAKE) -f CMakeFiles/enginesv.dir/build.make CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.provides.build
.PHONY : CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.provides

CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.provides.build: CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o


CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o: CMakeFiles/enginesv.dir/flags.make
CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o: enginesv_automoc.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/enginesv_automoc.cpp

CMakeFiles/enginesv.dir/enginesv_automoc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/enginesv.dir/enginesv_automoc.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/enginesv_automoc.cpp > CMakeFiles/enginesv.dir/enginesv_automoc.cpp.i

CMakeFiles/enginesv.dir/enginesv_automoc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/enginesv.dir/enginesv_automoc.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/enginesv_automoc.cpp -o CMakeFiles/enginesv.dir/enginesv_automoc.cpp.s

CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.requires:

.PHONY : CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.requires

CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.provides: CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.requires
	$(MAKE) -f CMakeFiles/enginesv.dir/build.make CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.provides.build
.PHONY : CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.provides

CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.provides.build: CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o


# Object files for target enginesv
enginesv_OBJECTS = \
"CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o" \
"CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o"

# External object files for target enginesv
enginesv_EXTERNAL_OBJECTS =

libenginesv.so: CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o
libenginesv.so: CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o
libenginesv.so: CMakeFiles/enginesv.dir/build.make
libenginesv.so: /usr/lib64/libQt5Core.so.5.5.1
libenginesv.so: CMakeFiles/enginesv.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library libenginesv.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/enginesv.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/enginesv.dir/build: libenginesv.so

.PHONY : CMakeFiles/enginesv.dir/build

CMakeFiles/enginesv.dir/requires: CMakeFiles/enginesv.dir/enginesv_dpx.cpp.o.requires
CMakeFiles/enginesv.dir/requires: CMakeFiles/enginesv.dir/enginesv_automoc.cpp.o.requires

.PHONY : CMakeFiles/enginesv.dir/requires

CMakeFiles/enginesv.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/enginesv.dir/cmake_clean.cmake
.PHONY : CMakeFiles/enginesv.dir/clean

CMakeFiles/enginesv.dir/depend:
	cd /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build /home/cabitech/flappyBird/FlappyBird-E135/newEngine_sv-build/CMakeFiles/enginesv.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/enginesv.dir/depend

