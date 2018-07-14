# min requirement

cmake_minimum_required(VERSION 3.1.0)

# project name
project (utilities)

set(PROJ_MAJOR_VERSION 0)
set(PROJ_MINOR_VERSION 1)
set(PROJ_PATCH_VERSION 0)
set(PROJ_VERSION
  ${PROJ_MAJOR_VERSION}.${PROJ_MINOR_VERSION}.${PROJ_PATCH_VERSION})

set(TARGET utilities)

# seting prefix
SET(CMAKE_INSTALL_PREFIX "./" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

# include paths
INCLUDE_DIRECTORIES(./include)

# get sources
# Find source files
file(GLOB SOURCES src/*.c src/*.cpp )

# executable name
#add_executable(myapp ${SOURCES})

# library name for lib project
add_library (${TARGET} SHARED ${SOURCES})

install(TARGETS ${TARGET} DESTINATION lib)
install(TARGETS ${TARGET} DESTINATION ../Package/Deliverable/artifacts)
#install(FILES pmaths.h DESTINATION include)

# setting out put paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
