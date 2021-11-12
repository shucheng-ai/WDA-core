#include "configurable.h"
#include "common/error.h"

namespace layout::configurable{

    py::object Configurable::python () const{
        py::dict ret;
        for (auto &entry : *this)
            ret[entry.first.c_str()] = entry.second->python();
        return py::make_tuple(ret, py::cast(this->order));
    }

    void Configurable::update (py::dict params){
        for (auto &var : *this){
            if (params.contains(var.first)){
                try{
                    var.second->update(params[var.first.c_str()]);
                    if (!var.second->check()) throw 0;
                }
                catch(...) {
                    raiseError(ParameterError, var.first);
                }
            }
        }
    }

    void Configurable::registeR (string const &name, int *value, string const &description){
        order.push_back(name);
        this->emplace(name, new IntVariable(name, value, description));
    }

    void Configurable::registeR (string const &name, int *value, pi range, string const &description){
        order.push_back(name);
        this->emplace(name, new RangedIntVariable(name, value, range, description));
    }

    void Configurable::registeR (string const &name, int *value, pi range, vector<int> const &choices, string const &description){
        order.push_back(name);
        this->emplace(name, new AlterIntVariable(name, value, range, choices, description));
    }

    void Configurable::registeR (string const &name, string *value, string const &description){
        order.push_back(name);
        this->emplace(name, new StrVariable(name, value, description));
    }

    void Configurable::registeR (string const &name, string *value, vector<string> const &choices, string const &description){
        order.push_back(name);
        this->emplace(name, new AlterStrVariable(name, value, choices, description));
    }

}
