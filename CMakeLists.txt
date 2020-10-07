CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

PROJECT(msgs-rtps LANGUAGES C CXX )

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE (${CMAKE_SOURCE_DIR}/cmake/utils.cmake)
INCLUDE(GenerateExportHeader)

ADD_PLATFORM_ARCH()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING 
     "Choose the type of build, options are : None Debug Release RelWithDebInfo MinSizeRel."
     FORCE)
ENDIF()

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

SET(PROJECT_CMAKE_ROOT "${CMAKE_SOURCE_DIR}/cmake")

SET(CMAKE_MODULE_PATH "${PROJECT_CMAKE_ROOT};${CMAKE_MODULE_PATH}")

# The version number.
SET (MSGS_VERSION_MAJOR 0)
SET (MSGS_VERSION_MINOR 1)
SET (MSGS_VERSION_PATCH 0)

set(MSGS_VERSION ${MSGS_VERSION_MAJOR}.${MSGS_VERSION_MINOR}.${MSGS_VERSION_PATCH})

set(FASTRTPSHOME /opt/eprosima)
set(fastcdr_DIR ${FASTRTPSHOME}/fastcdr/lib/cmake/fastcdr)
set(fastrtps_DIR ${FASTRTPSHOME}/fastdds/share/fastrtps/cmake)
set(foonathan_memory_DIR ${FASTRTPSHOME}/foonathan_memory_vendor/lib/foonathan_memory/cmake)

find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)
find_package(foonathan_memory REQUIRED)

# All the below packages will be packed from vcpkg
find_package(pugixml CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(tinyxml2 CONFIG REQUIRED)

# Usage report verbosity
set(MSGS_USAGE_REPORT_VERBOSITY "0" CACHE STRING "Verbosity of OptiX usage report (0 disables reporting).")
set_property(CACHE MSGS_USAGE_REPORT_VERBOSITY PROPERTY STRINGS 0 1 2 3)

option(MSGS_USE_DEBUG_EXCEPTIONS "Enables advanced exception handling and error checking for debugging purposes." ON)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

# Export no symbols by default (if the compiler supports it).
# This makes e.g. GCC's "visibility behavior" consistent with MSVC's. 
# On Windows/MSVC this is a noop.
#set(CMAKE_C_VISIBILITY_PRESET hidden)
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )

IF (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250")
ENDIF(MSVC)

# Always include the source and build directories in the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

IF (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  add_definitions(-DNDEBUG)
ENDIF()

ADD_SUBDIRECTORY(msgs)
#ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(appln)
