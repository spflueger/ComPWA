#######################
# ComPWA Core library #
#######################

set(srcs 
	Particle.cpp
)
set(headers
	Particle.hpp
	Utils.hpp
)

add_library(Core
  SHARED ${lib_srcs} ${lib_headers}
  $<TARGET_OBJECTS:easyloggingpp_obj>
)

target_include_directories(Core
  PUBLIC ${ELPP_INCLUDE_DIRS} Boost::serialization
)

target_link_libraries(Core
  PUBLIC Boost::serialization
)

install(TARGETS Core
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/Core
)

add_subdirectory(FunctionGraph)
