#pragma once
#include "common/geometry.hpp"
#include "configurable/configurable.h"
#include "common/error.h"
#include "item/item.h"

namespace layout::method{
	struct LayoutResult;
}

namespace layout::storage{

    using method::LayoutResult;
    using configurable::Configurable;
    using item::Item;

    using PackageModel = RecursiveBox3D;

    struct SkeletonModel : vector<unique_ptr<Item>>{
        
        py::object python () const{
            py::dict ret;
            for (auto const &it : *this)
                ret[it->info.c_str()] = it->python();
            return ret;
        }

        void unmap (const Local2D &region){
            for (auto &it : *this)
                it->unmap(region);
        }

        Box bbox () const{
            Box ret;
            for (auto const &it : *this)
                ret.expand(it->bbox());
            return ret;
        }
    };

    enum Storage{
        UPRIGHT,
        PACKAGE,
        UNDERPASS_PACKAGE,
        EMPTY
    };

    enum Direction{
        LEFTWARD = 1,
        RIGHTWARD = -1
    };

    enum Coordinate{
        HORIZONTAL = 0,
        VERTICAL = 1
    };

    class StorageModel{
        protected:
            static const int STORAGE_MAX;

            Configurable parameters;

        public :

            string info;
            int layers, allow_involved_column;
            //parameters for horizontal DP
            int storage_depth, depth, passage_width, beam_depth, storage_gap;
            //parameters for vertical DP
            int upright_width, package_width;
            int pallet_per_face, max_pallet_per_face, underpass_pallet_per_face;

            int moving_path_width, shortcut_width;
            int underpass_layers_for_moving_path, underpass_layers_for_shortcut;
            //int adjust_gap_for_underpass;
            //===================
            void updateParameters (py::dict params) {parameters.update(params);}
            py::object showParameters () const{return parameters.python();}
            //==================
            virtual void extractParameters() = 0;
            py::object showDerived() const;
            virtual void generateResult(const LayoutResult &plan, pair<PackageModel, SkeletonModel> *result) const = 0;

            StorageModel () {};
            virtual ~StorageModel () {}

            static map<string, function<StorageModel *()>> & registry(){   
                static map<string, function<StorageModel *()>> impl;
                return impl;
            }   

            struct Registrar{   
                Registrar (string const &name, function<StorageModel *()> create){   
                    StorageModel::registry()[name] = create;
                }   
            };  

            static StorageModel *create (string const &name) {
                auto it = registry().find(name);
                if (it == registry().end())
                    raiseError(StorageError, "unavailable type of storage : " + name);
                return it->second();
            }
    };  
}

namespace layout{
    py::object rackList();
    py::object rackSetting(string const &type);
}
