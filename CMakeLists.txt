cmake_minimum_required(VERSION 2.6)

PROJECT(littlelog)

set(CXX_FLAGS
 -g
 # -DVALGRIND
 # -DMUDUO_STD_STRING
#  -D_FILE_OFFSET_BITS=64
#  -Wall
#  -Wextra
#  -Werror
#  -Wconversion
#  -Wno-unused-parameter
#  -Wold-style-cast
#  -Woverloaded-virtual
#  -Wpointer-arith
#  -Wshadow
#  -Wwrite-strings
#  -march=native
 # -MMD
 #-std=c++xx
 #-rdynamic
 )

 set(CMAKE_CXX_FLAGS ${CXX_FLAGS})

 set(CMAKE_CXX_COMPLIER "g++")
 set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
 set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

 include_directories(${PROJECT_SOURCE_DIR}/src)

 add_subdirectory(src)
 add_subdirectory(test)
 
 
 


