#pragma once
#include "common/error.h"
#include "room/room.h"
#include "storage/storage.h"

namespace layout::method{

    struct HoriResult{ //每一个单元最多model->depth列
        int left, right, direction;
        HoriResult () {}
        HoriResult (int _left, int _right, int _direction) : left(_left), right(_right), direction(_direction) {}
    };

    struct VertiResult{
        int bottom, top, type;
        VertiResult (int _bottom, int _top, int _type) : bottom(_bottom), top(_top), type(_type){}
    };

    struct IndividualResult : HoriResult, vector<VertiResult>{ //每一个单元一列
        IndividualResult (int _left, int _right, int _direction) : HoriResult(_left, _right, _direction){}
    };

    struct LayoutResult{
        vector<IndividualResult> rows;
        vector<pi> hpoints, vpoints;

        void clear (){
            rows.clear(); hpoints.clear(); vpoints.clear();
        }
    };

    using storage::StorageModel;
    using room::ColisionSystem;
    using room::ConnectionSystem;
    using configurable::Configurable;

    class LayoutMethod{
        protected :
            Size_<2> size;
            const StorageModel *model;
            ColisionSystem const *col;
            ConnectionSystem const *con;
            LayoutResult *result;

            Configurable parameters;

            static map<string, function<LayoutMethod *()>> & registry (){   
                static map<string, function<LayoutMethod *()>> impl;
                return impl;
            }   

        public :
            py::object showParameters () const{return parameters.python();}

            virtual void solve(Size_<2> const &_size, const StorageModel &_model, const ColisionSystem &_col, const ConnectionSystem &_con, py::dict info, LayoutResult* const _result) = 0;

            struct Registrar{   
                Registrar (string const &name, function<LayoutMethod *()> create){   
                    LayoutMethod::registry()[name] = create;
                }   
            }; 

            static LayoutMethod *create (string const &name) {
                auto it = registry().find(name);
                if (it == registry().end())
                    raiseError(MethodError, "unavailable type of layout method : " + name);
                return it->second();
            }
    };

    class RegionConnection{
        protected :
            Configurable parameters;
        
            static map<string, function<RegionConnection *()>> & registry (){
                static map<string, function<RegionConnection *()>> impl;
                return impl;
            }
        public :

            virtual py::dict solve(py::dict connection_item, const Local2D &region, const StorageModel &_model, py::dict info) = 0;

            py::object showParameters () {return parameters.python();};

            struct Registrar{   
                Registrar (string const &name, function<RegionConnection *()> create){   
                    RegionConnection::registry()[name] = create;
                }   
            }; 

            static RegionConnection *create (string const &name) {
                auto it = registry().find(name);
                if (it == registry().end())
                    raiseError(MethodError, "unavailable type of region connection  method : " + name);
                return it->second();
            }
    };
}

namespace layout{
    py::object generate_plan_within_region(py::dict _room, pyBox _region, py::dict connection_item, py::dict history, string const &type, py::dict params, py::dict info);
    py::object algorithmSetting(string const &name);
    py::object regionConnectionSetting(string const &name);
    py::object region_connection(pyBox _region, py::dict connection_item, string const &type, py::dict params, py::dict info);
}
