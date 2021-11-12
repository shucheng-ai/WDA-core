#include "storage/storage.h"
#include "common/error.h"
#include "room/room.h"
#include "method/method.h"
using namespace layout;

namespace{
    Qlog _log("layout");
}

void setLogPath(string const &name){
    _log.setLogPath(name);
}

PYBIND11_MODULE(layout, m){

    m.doc() = "I'm cute! Please give me money.";
    m.def("set_log_path", &setLogPath, "");
    m.def("base_rack_list", &rackList, R"pbdoc(
        Base rack list.
    )pbdoc");
    m.def("base_rack_setting", &rackSetting, "");
    m.def("base_region_setting", &roomSetting, "");
    m.def("base_algorithm_setting", &algorithmSetting, "");
    m.def("generate_plan_within_region", &generate_plan_within_region, "");
    m.def("convert_wall", &convert_wall, "");
    m.def("region_in_room", &region_in_room, "");
    m.def("region_connection", &region_connection, "");
    m.def("base_region_connection_setting", &regionConnectionSetting, "");
    m.def("convert_connection_item", &connectionConvert, "");
    errorDef(m.def_submodule("error", "submodule of error"));
}

