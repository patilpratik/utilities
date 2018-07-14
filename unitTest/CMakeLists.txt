# min requirement

cmake_minimum_required(VERSION 3.1.0)

# setting cpp 11 
set (CMAKE_CXX_STANDARD 11)

set (TEST_TARGET utilities)
# project name
project (ut_${TEST_TARGET})

# seting prefix
SET(CMAKE_INSTALL_PREFIX "./" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

# include paths
INCLUDE_DIRECTORIES(../include)


# get sources
# Find source files
file(GLOB SOURCES *.cpp)

# executable name
add_executable( ut_${TEST_TARGET} ${SOURCES}) 

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#find_library(	PMATHS_LIB 
#		NAMES utilities 
#		HINTS ../../build)


# set the path to the library folder
#link_directories(../../build/lib)

# adding googletest library to link
add_library(gtest STATIC IMPORTED) # or STATIC instead of SHARED
set_target_properties(gtest PROPERTIES
# Update this location to point to libgtest.a
  IMPORTED_LOCATION "../../../googleTest/googletest/googlemock/gtest/libgtest.a"
)

# adding library to test
add_library(${TEST_TARGET} SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(${TEST_TARGET} PROPERTIES
  IMPORTED_LOCATION "../../build/lib/lib${TEST_TARGET}.so"
  INTERFACE_INCLUDE_DIRECTORIES "../../include"
)


# link the libraries to the executable
target_link_libraries( ut_${TEST_TARGET} Threads::Threads)
target_link_libraries ( ut_${TEST_TARGET} ${TEST_TARGET})
target_link_libraries ( ut_${TEST_TARGET} gtest)

# library name for lib project
#add_library (pmaths STATIC ${SOURCES})

#install(TARGETS pmaths DESTINATION lib)
#install(FILES pmaths.h DESTINATION include)

# setting out put paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
