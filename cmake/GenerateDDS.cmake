cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(FASTRTPSHOME $ENV{FASTRTPSHOME})

#set(fastcdr_DIR ${FASTRTPSHOME}/lib/cmake/fastcdr)
#set(fastrtps_DIR ${FASTRTPSHOME}/share/fastrtps/cmake)
#set(foonathan_memory_DIR ${FASTRTPSHOME}/share/foonathan_memory/cmake)

find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

find_path(FASTDDS_INCL_DIR 
        NAMES "fastrtps/Domain.h"
        PATHS ${FASTRTPSHOME}/fastdds/include
        ${FASTRTPSHOME}/fastrtps/include
        ${FASTRTPSHOME}/include
        /usr/bin/include
)

include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(--std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()


MACRO(add_message msg_lib msg_file)
	MESSAGE( STATUS "Configuring msg file : " ${msg_file})
    get_filename_component(msg_name ${msg_file} NAME_WE)
    set(GEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${msg_name})

    set(cpp_file ${GEN_OUTPUT_DIR}/${msg_name}.cxx)
    set(header_file ${GEN_OUTPUT_DIR}/${msg_name}.h)
    set(cpp_file_types ${GEN_OUTPUT_DIR}/${msg_name}PubSubTypes.cxx)
    set(header_file_types ${GEN_OUTPUT_DIR}/${msg_name}PubSubTypes.h)
    # BEGIN Generated using option example CMake for generating [Subscriber, Publisher].[h,cxx] 
    set(cpp_file_publisher ${GEN_OUTPUT_DIR}/${msg_name}Publisher.cxx)
    set(header_file_publisher ${GEN_OUTPUT_DIR}/${msg_name}Publisher.h)
    set(cpp_file_subscriber ${GEN_OUTPUT_DIR}/${msg_name}Subscriber.cxx)
    set(header_file_subscriber ${GEN_OUTPUT_DIR}/${msg_name}Subscriber.h)
    # END Generated using option example CMake for generating [Subscriber, Publisher].[h,cxx] 

    # Adding example CMake for generating [Subscriber, Publisher].[h,cxx] 
    add_custom_command(
        OUTPUT ${cpp_file} ${header_file} ${cpp_file_types} ${header_file_types} ${cpp_file_publisher} ${header_file_publisher} ${cpp_file_subscriber} ${header_file_subscriber}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_OUTPUT_DIR}
        COMMAND fastddsgen -d ${GEN_OUTPUT_DIR} -example CMake -replace ${msg_file} 
        DEPENDS ${msg_file}
        COMMENT "Executing Fastddsgen on Message File ${msg_file}"
        VERBATIM 
    )
    add_library(${msg_name} 
                ${cpp_file} ${header_file} 
                ${cpp_file_types} ${header_file_types}
                ${cpp_file_publisher} ${header_file_publisher}
                ${cpp_file_subscriber} ${header_file_subscriber}
    )
    target_link_libraries(${msg_name} fastrtps)
    target_include_directories(${msg_name} PUBLIC 
        ${GEN_OUTPUT_DIR}
        ${FASTDDS_INCL_DIR} 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${msg_name}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${msg_name}>
    )
    set_target_properties(${msg_name} PROPERTIES FOLDER msgs)

    set(msg_lib ${msg_name})

ENDMACRO()
