#include <pybind11/pybind11.h>
#include "error.h"

namespace layout{

    void errorDef(py::module m){ 

        py::register_exception<UnknownError>(m, "UnknownError");
        py::class_<UnknownError>(m, "UnknownWarning").def("__repr__", &UnknownError::what);
        py::register_exception<StorageError>(m, "StorageError");
        py::class_<StorageError, UnknownError>(m, "StorageWarning").def("__repr__", &StorageError::what);
        py::register_exception<ParameterError>(m, "ParameterError");
        py::class_<ParameterError, UnknownError>(m, "ParameterWarning").def("__repr__", &ParameterError::what);
        py::register_exception<MethodError>(m, "MethodError");
        py::class_<MethodError, UnknownError>(m, "MethodWarning").def("__repr__", &MethodError::what);
        py::register_exception<InterfaceError>(m, "InterfaceError");
        py::class_<InterfaceError, UnknownError>(m, "InterfaceWarning").def("__repr__", &InterfaceError::what);

        m.def("warning_list", [](bool clear){
            py::list ret;
            for (auto &warning : warning_list) ret.append(*warning.get());
            if (clear) warning_list.clear();
            return ret;
        },
        "clear"_a = 1
        );                

        m.def("clear_warning_list", []{
           warning_list.clear(); 
        });
    }

    vector<unique_ptr<UnknownError>> warning_list;

}
