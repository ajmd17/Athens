# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.6.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.6.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/andrew/ace-lang

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/andrew/ace-lang/bin

# Include any dependencies generated for this target.
include src/ace/CMakeFiles/ace.dir/depend.make

# Include the progress variables for this target.
include src/ace/CMakeFiles/ace.dir/progress.make

# Include the compile flags for this target's objects.
include src/ace/CMakeFiles/ace.dir/flags.make

src/ace/CMakeFiles/ace.dir/api.cpp.o: src/ace/CMakeFiles/ace.dir/flags.make
src/ace/CMakeFiles/ace.dir/api.cpp.o: ../src/ace/api.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/andrew/ace-lang/bin/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/ace/CMakeFiles/ace.dir/api.cpp.o"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ace.dir/api.cpp.o -c /Users/andrew/ace-lang/src/ace/api.cpp

src/ace/CMakeFiles/ace.dir/api.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ace.dir/api.cpp.i"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/andrew/ace-lang/src/ace/api.cpp > CMakeFiles/ace.dir/api.cpp.i

src/ace/CMakeFiles/ace.dir/api.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ace.dir/api.cpp.s"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/andrew/ace-lang/src/ace/api.cpp -o CMakeFiles/ace.dir/api.cpp.s

src/ace/CMakeFiles/ace.dir/api.cpp.o.requires:

.PHONY : src/ace/CMakeFiles/ace.dir/api.cpp.o.requires

src/ace/CMakeFiles/ace.dir/api.cpp.o.provides: src/ace/CMakeFiles/ace.dir/api.cpp.o.requires
	$(MAKE) -f src/ace/CMakeFiles/ace.dir/build.make src/ace/CMakeFiles/ace.dir/api.cpp.o.provides.build
.PHONY : src/ace/CMakeFiles/ace.dir/api.cpp.o.provides

src/ace/CMakeFiles/ace.dir/api.cpp.o.provides.build: src/ace/CMakeFiles/ace.dir/api.cpp.o


src/ace/CMakeFiles/ace.dir/main.cpp.o: src/ace/CMakeFiles/ace.dir/flags.make
src/ace/CMakeFiles/ace.dir/main.cpp.o: ../src/ace/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/andrew/ace-lang/bin/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/ace/CMakeFiles/ace.dir/main.cpp.o"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ace.dir/main.cpp.o -c /Users/andrew/ace-lang/src/ace/main.cpp

src/ace/CMakeFiles/ace.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ace.dir/main.cpp.i"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/andrew/ace-lang/src/ace/main.cpp > CMakeFiles/ace.dir/main.cpp.i

src/ace/CMakeFiles/ace.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ace.dir/main.cpp.s"
	cd /Users/andrew/ace-lang/bin/src/ace && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/andrew/ace-lang/src/ace/main.cpp -o CMakeFiles/ace.dir/main.cpp.s

src/ace/CMakeFiles/ace.dir/main.cpp.o.requires:

.PHONY : src/ace/CMakeFiles/ace.dir/main.cpp.o.requires

src/ace/CMakeFiles/ace.dir/main.cpp.o.provides: src/ace/CMakeFiles/ace.dir/main.cpp.o.requires
	$(MAKE) -f src/ace/CMakeFiles/ace.dir/build.make src/ace/CMakeFiles/ace.dir/main.cpp.o.provides.build
.PHONY : src/ace/CMakeFiles/ace.dir/main.cpp.o.provides

src/ace/CMakeFiles/ace.dir/main.cpp.o.provides.build: src/ace/CMakeFiles/ace.dir/main.cpp.o


# Object files for target ace
ace_OBJECTS = \
"CMakeFiles/ace.dir/api.cpp.o" \
"CMakeFiles/ace.dir/main.cpp.o"

# External object files for target ace
ace_EXTERNAL_OBJECTS =

src/ace/ace: src/ace/CMakeFiles/ace.dir/api.cpp.o
src/ace/ace: src/ace/CMakeFiles/ace.dir/main.cpp.o
src/ace/ace: src/ace/CMakeFiles/ace.dir/build.make
src/ace/ace: src/ace-c/libace-c.a
src/ace/ace: src/ace-vm/libace-vm.a
src/ace/ace: src/ace/CMakeFiles/ace.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/andrew/ace-lang/bin/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ace"
	cd /Users/andrew/ace-lang/bin/src/ace && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ace.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/ace/CMakeFiles/ace.dir/build: src/ace/ace

.PHONY : src/ace/CMakeFiles/ace.dir/build

src/ace/CMakeFiles/ace.dir/requires: src/ace/CMakeFiles/ace.dir/api.cpp.o.requires
src/ace/CMakeFiles/ace.dir/requires: src/ace/CMakeFiles/ace.dir/main.cpp.o.requires

.PHONY : src/ace/CMakeFiles/ace.dir/requires

src/ace/CMakeFiles/ace.dir/clean:
	cd /Users/andrew/ace-lang/bin/src/ace && $(CMAKE_COMMAND) -P CMakeFiles/ace.dir/cmake_clean.cmake
.PHONY : src/ace/CMakeFiles/ace.dir/clean

src/ace/CMakeFiles/ace.dir/depend:
	cd /Users/andrew/ace-lang/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/andrew/ace-lang /Users/andrew/ace-lang/src/ace /Users/andrew/ace-lang/bin /Users/andrew/ace-lang/bin/src/ace /Users/andrew/ace-lang/bin/src/ace/CMakeFiles/ace.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/ace/CMakeFiles/ace.dir/depend
