pybind11_add_module(test_geometry test_geometry.cpp)
target_link_libraries(test_geometry PRIVATE glog)
pybind11_add_module(test_basic test_basic.cpp)
target_link_libraries(test_basic PRIVATE glog)
