#######################
# ComPWA Core library #
#######################

set(srcs 
	FunctionGraph.cpp
	FunctionWrapper.cpp
	VectorizedEvaluator.cpp
)
set(headers
	FunctionGraph.hpp
	FunctionWrapper.hpp
	VectorizedEvaluator.hpp
)

add_library(FunctionGraph
  SHARED ${srcs} ${headers}
  $<TARGET_OBJECTS:easyloggingpp_obj>
)

target_include_directories(FunctionGraph
  PUBLIC ${ELPP_INCLUDE_DIRS}
)

target_link_libraries(FunctionGraph
  PUBLIC Boost::serialization pstl::ParallelSTL
)

install(TARGETS FunctionGraph
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/Core
)
