#-------------------------------------------------------------------------------
MACRO (SET_GLOBAL_VARIABLE name value)
  set( _value_list ${value} ${ARGN} ) # support if values passed is list, else only one value is passed over
  SET (${name} ${_value_list} CACHE INTERNAL "Used to pass variables between directories" FORCE)
ENDMACRO (SET_GLOBAL_VARIABLE)
#-------------------------------------------------------------------------------
MACRO(SUBDIRLIST RESULT CURDIR)
  FILE(GLOB children RELATIVE ${CURDIR} ${CURDIR}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${CURDIR}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${RESULT} ${dirlist})
ENDMACRO()
#-------------------------------------------------------------------------------
MACRO (ADD_PLATFORM_ARCH)
    set(ARCH "x32")
    IF (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(ARCH "x64")
    ENDIF()

    if(APPLE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -Wno-deprecated-declarations")
    endif()

    IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(PLATFORM "win32")
        add_definitions(-D_WIN32)
        add_definitions(-DNOMINMAX)
    ENDIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
       # Mac OS X specific code
       set (CMAKE_MACOSX_RPATH 1)
       add_definitions(-DMAC_OSX)
       set(PLATFORM "darwin")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
       # Linux specific code
       add_definitions(-DLINUX)
       set(PLATFORM "linux")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
ENDMACRO()
