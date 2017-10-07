file (GLOB_RECURSE gs_control *)

add_library(gs_control  SHARED  ${gs_control})

set_target_properties(gs_control PROPERTIES
			VERSION 0.1
			SUBVERSION 1)

target_link_libraries(gs_control external_control orbit_simulator)


add_subdirectory(orbit_propagator)
add_subdirectory(external_control)
