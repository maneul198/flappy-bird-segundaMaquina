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
CMAKE_SOURCE_DIR = /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build

# Include any dependencies generated for this target.
include CMakeFiles/digitalio_dpx.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/digitalio_dpx.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/digitalio_dpx.dir/flags.make

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o: CMakeFiles/digitalio_dpx.dir/flags.make
CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o: ../digitalinput.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitalinput.cpp

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitalinput.cpp > CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.i

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitalinput.cpp -o CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.s

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.requires:

.PHONY : CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.requires

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.provides: CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.requires
	$(MAKE) -f CMakeFiles/digitalio_dpx.dir/build.make CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.provides.build
.PHONY : CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.provides

CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.provides.build: CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o


CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o: CMakeFiles/digitalio_dpx.dir/flags.make
CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o: ../digitaloutput.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitaloutput.cpp

CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitaloutput.cpp > CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.i

CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/digitaloutput.cpp -o CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.s

CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.requires:

.PHONY : CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.requires

CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.provides: CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.requires
	$(MAKE) -f CMakeFiles/digitalio_dpx.dir/build.make CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.provides.build
.PHONY : CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.provides

CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.provides.build: CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o


CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o: CMakeFiles/digitalio_dpx.dir/flags.make
CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o: ../watchioports.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/watchioports.cpp

CMakeFiles/digitalio_dpx.dir/watchioports.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/digitalio_dpx.dir/watchioports.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/watchioports.cpp > CMakeFiles/digitalio_dpx.dir/watchioports.cpp.i

CMakeFiles/digitalio_dpx.dir/watchioports.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/digitalio_dpx.dir/watchioports.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/watchioports.cpp -o CMakeFiles/digitalio_dpx.dir/watchioports.cpp.s

CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.requires:

.PHONY : CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.requires

CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.provides: CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.requires
	$(MAKE) -f CMakeFiles/digitalio_dpx.dir/build.make CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.provides.build
.PHONY : CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.provides

CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.provides.build: CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o


CMakeFiles/digitalio_dpx.dir/main.cpp.o: CMakeFiles/digitalio_dpx.dir/flags.make
CMakeFiles/digitalio_dpx.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/digitalio_dpx.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/digitalio_dpx.dir/main.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/main.cpp

CMakeFiles/digitalio_dpx.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/digitalio_dpx.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/main.cpp > CMakeFiles/digitalio_dpx.dir/main.cpp.i

CMakeFiles/digitalio_dpx.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/digitalio_dpx.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/main.cpp -o CMakeFiles/digitalio_dpx.dir/main.cpp.s

CMakeFiles/digitalio_dpx.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/digitalio_dpx.dir/main.cpp.o.requires

CMakeFiles/digitalio_dpx.dir/main.cpp.o.provides: CMakeFiles/digitalio_dpx.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/digitalio_dpx.dir/build.make CMakeFiles/digitalio_dpx.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/digitalio_dpx.dir/main.cpp.o.provides

CMakeFiles/digitalio_dpx.dir/main.cpp.o.provides.build: CMakeFiles/digitalio_dpx.dir/main.cpp.o


CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o: CMakeFiles/digitalio_dpx.dir/flags.make
CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o: digitalio_dpx_automoc.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o -c /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/digitalio_dpx_automoc.cpp

CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/digitalio_dpx_automoc.cpp > CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.i

CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/digitalio_dpx_automoc.cpp -o CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.s

CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.requires:

.PHONY : CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.requires

CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.provides: CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.requires
	$(MAKE) -f CMakeFiles/digitalio_dpx.dir/build.make CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.provides.build
.PHONY : CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.provides

CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.provides.build: CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o


# Object files for target digitalio_dpx
digitalio_dpx_OBJECTS = \
"CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o" \
"CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o" \
"CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o" \
"CMakeFiles/digitalio_dpx.dir/main.cpp.o" \
"CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o"

# External object files for target digitalio_dpx
digitalio_dpx_EXTERNAL_OBJECTS =

libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/main.cpp.o
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/build.make
libdigitalio_dpx.so: /usr/lib64/libQt5Core.so.5.5.1
libdigitalio_dpx.so: CMakeFiles/digitalio_dpx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX shared library libdigitalio_dpx.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/digitalio_dpx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/digitalio_dpx.dir/build: libdigitalio_dpx.so

.PHONY : CMakeFiles/digitalio_dpx.dir/build

CMakeFiles/digitalio_dpx.dir/requires: CMakeFiles/digitalio_dpx.dir/digitalinput.cpp.o.requires
CMakeFiles/digitalio_dpx.dir/requires: CMakeFiles/digitalio_dpx.dir/digitaloutput.cpp.o.requires
CMakeFiles/digitalio_dpx.dir/requires: CMakeFiles/digitalio_dpx.dir/watchioports.cpp.o.requires
CMakeFiles/digitalio_dpx.dir/requires: CMakeFiles/digitalio_dpx.dir/main.cpp.o.requires
CMakeFiles/digitalio_dpx.dir/requires: CMakeFiles/digitalio_dpx.dir/digitalio_dpx_automoc.cpp.o.requires

.PHONY : CMakeFiles/digitalio_dpx.dir/requires

CMakeFiles/digitalio_dpx.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/digitalio_dpx.dir/cmake_clean.cmake
.PHONY : CMakeFiles/digitalio_dpx.dir/clean

CMakeFiles/digitalio_dpx.dir/depend:
	cd /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build /home/cabitech/flappyBird/FlappyBird-E135/newDigitalioLib/build/CMakeFiles/digitalio_dpx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/digitalio_dpx.dir/depend

