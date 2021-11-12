#include "common/common.h"

using namespace layout;

void test1(pi temp){
    std::cout << temp.first << " " << temp.second;
}

py::dict test2(py::dict temp){
    temp["hello"] = 1;
    return temp;
}

PYBIND11_MODULE(test_basic, m){
    m.doc() = "test";
    m.def("test1", &test1);
    m.def("test2", &test2);
}
