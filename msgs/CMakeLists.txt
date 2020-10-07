include(${CMAKE_SOURCE_DIR}/cmake/GenerateDDS.cmake)

#file(GLOB_RECURSE msg_files "*.msg")
file(GLOB msg_files "*.msg")

set(msglibs "")

foreach(msg_file ${msg_files})
    add_message(msg_lib ${msg_file})
    list(APPEND msglibs ${msg_lib})
    set_target_properties (${msg_lib} PROPERTIES FOLDER ${PROJECT_NAME}/msgs)
endforeach(msg_file)

set_global_variable(MSG_LIBRARIES ${msglibs})

add_subdirectory(msglib)