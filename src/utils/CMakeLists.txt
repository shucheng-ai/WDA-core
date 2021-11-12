pybind11_add_module(geometry geometry.cpp)
set_target_properties(geometry PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
