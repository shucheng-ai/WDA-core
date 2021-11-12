#include "configurable.h"

namespace layout::configurable{

    py::dict Variable::python () const {
        py::dict ret;
        ret["name"] = name;
        ret["description"] = description;
        return ret;
    }

    void Variable::update (py::dict params){
        name = params["name"].cast<string>();
        description = params["description"].cast<string>();
    }

    Variable::Variable (string _name, const string &_description) : name(_name), description(_description){}

    py::dict IntVariable::python () const{
        py::dict ret = Variable::python();
        ret["value"] = value;
        return ret;
    }

    void IntVariable::update (py::dict params){
        Variable::update(params);
        *value = params["value"].cast<int>();
    }

    int &IntVariable::asInt (){
        return *value;
    }

    IntVariable::IntVariable(string _name, int *_value, const string &_description) : Variable(_name, _description) , value(_value) {}

    py::dict RangedIntVariable::python () const{
        py::dict ret = IntVariable::python();
        ret["range"] = range;
        return ret;
    }

    void RangedIntVariable::update (py::dict params){
        IntVariable::update(params);
        range = params["range"].cast<pi>();
    }

    RangedIntVariable::RangedIntVariable (string _name, int *_value, pi _range, const string &_description) : IntVariable(_name, _value, _description), range(_range) {}

    bool RangedIntVariable::check() const{
        return range.first <= *value && *value <= range.second;
    }

    py::dict AlterIntVariable::python () const{
        py::dict ret = RangedIntVariable::python();
        ret["choices"] = choices;
        return ret;
    }

    void AlterIntVariable::update (py::dict params){
        RangedIntVariable::update(params); 
        choices = params["choices"].cast<vector<int>>();
    }

    AlterIntVariable::AlterIntVariable (string _name, int *_value, pi _range, const vector<int>& _choices, const string &_description) : RangedIntVariable(_name, _value, _range, _description), choices(_choices) {}

    bool AlterIntVariable::check () const{
        bool flag = 0;
        for (auto &c : choices) flag |= c == *value;
        for (auto &c : choices) flag &= range.first <= c && c <= range.second;
        return flag;
    }

    py::dict StrVariable::python() const{
        py::dict ret = Variable::python();
        ret["value"] = value;
        return ret;
    }

    void StrVariable::update (py::dict params){
        Variable::update(params);
        *value = params["value"].cast<string>();
    }

    StrVariable::StrVariable (string _name, string *_value, const string &_description) : Variable(_name, _description), value(_value) {}

    string &StrVariable::asString(){
        return *value;
    }

    py::dict AlterStrVariable::python () const{
        py::dict ret = StrVariable::python();
        ret["choices"] = choices;
        return ret;
    }

    void AlterStrVariable::update (py::dict params){
        StrVariable::update(params); 
        choices = params["choices"].cast<vector<string>>();
    }

    AlterStrVariable::AlterStrVariable (string _name, string *_value, const vector<string>& _choices, const string &_description) : StrVariable(_name, _value, _description), choices(_choices) {}

    bool AlterStrVariable::check () const{
        bool flag = 0;
        for (auto &c : choices) flag |= c == *value;
        return flag;
    }

}
