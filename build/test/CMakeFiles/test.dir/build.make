# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/presonal/project/LittleLog/LittleLog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/presonal/project/LittleLog/LittleLog/build

# Include any dependencies generated for this target.
include test/CMakeFiles/test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test.dir/flags.make

test/CMakeFiles/test.dir/test.cpp.o: test/CMakeFiles/test.dir/flags.make
test/CMakeFiles/test.dir/test.cpp.o: ../test/test.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/presonal/project/LittleLog/LittleLog/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/CMakeFiles/test.dir/test.cpp.o"
	cd /home/presonal/project/LittleLog/LittleLog/build/test && /usr/lib/ccache/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/test.dir/test.cpp.o -c /home/presonal/project/LittleLog/LittleLog/test/test.cpp

test/CMakeFiles/test.dir/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/test.cpp.i"
	cd /home/presonal/project/LittleLog/LittleLog/build/test && /usr/lib/ccache/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/presonal/project/LittleLog/LittleLog/test/test.cpp > CMakeFiles/test.dir/test.cpp.i

test/CMakeFiles/test.dir/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/test.cpp.s"
	cd /home/presonal/project/LittleLog/LittleLog/build/test && /usr/lib/ccache/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/presonal/project/LittleLog/LittleLog/test/test.cpp -o CMakeFiles/test.dir/test.cpp.s

test/CMakeFiles/test.dir/test.cpp.o.requires:
.PHONY : test/CMakeFiles/test.dir/test.cpp.o.requires

test/CMakeFiles/test.dir/test.cpp.o.provides: test/CMakeFiles/test.dir/test.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/test.dir/build.make test/CMakeFiles/test.dir/test.cpp.o.provides.build
.PHONY : test/CMakeFiles/test.dir/test.cpp.o.provides

test/CMakeFiles/test.dir/test.cpp.o.provides.build: test/CMakeFiles/test.dir/test.cpp.o

# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/test.cpp.o"

# External object files for target test
test_EXTERNAL_OBJECTS =

bin/test: test/CMakeFiles/test.dir/test.cpp.o
bin/test: test/CMakeFiles/test.dir/build.make
bin/test: lib/liblittlelog.a
bin/test: test/CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../bin/test"
	cd /home/presonal/project/LittleLog/LittleLog/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test.dir/build: bin/test
.PHONY : test/CMakeFiles/test.dir/build

test/CMakeFiles/test.dir/requires: test/CMakeFiles/test.dir/test.cpp.o.requires
.PHONY : test/CMakeFiles/test.dir/requires

test/CMakeFiles/test.dir/clean:
	cd /home/presonal/project/LittleLog/LittleLog/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test.dir/clean

test/CMakeFiles/test.dir/depend:
	cd /home/presonal/project/LittleLog/LittleLog/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/presonal/project/LittleLog/LittleLog /home/presonal/project/LittleLog/LittleLog/test /home/presonal/project/LittleLog/LittleLog/build /home/presonal/project/LittleLog/LittleLog/build/test /home/presonal/project/LittleLog/LittleLog/build/test/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test.dir/depend

