file (GLOB_RECURSE external_control *)

add_library(external_control  SHARED  ${external_control})

set_target_properties(external_control PROPERTIES
            VERSION 0.1
            SUBVERSION 1)

target_link_libraries(external_control)
