file (GLOB_RECURSE orbit_simulator *)

add_library(orbit_simulator  SHARED  ${orbit_simulator})

set_target_properties(orbit_simulator PROPERTIES
			VERSION 0.1
			SUBVERSION 1)

target_link_libraries(orbit_simulator)
