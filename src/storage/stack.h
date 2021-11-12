#pragma once
#include "storage.h"

#define for_stack_height(i)  for (int i = 0; i < _package_layers; i += _package_height)

namespace layout::storage{

    class Stack : public StorageModel{
        protected :
            int _package_height = 1500;
            int _package_width = 1000;
            int _package_depth = 1200;
            int _package_layers = 1;
            int _package_gap = 100;
            int _aisle_width = 1500;
            int _moving_path_width = 2800;
            int _shortcut_width = 900;

            int _stack_depth = 1;
        public :
            Stack (){
                parameters.registeR("package_height", &_package_height, make_pair(1, STORAGE_MAX), "包裹的高度");
                parameters.registeR("package_width", &_package_width, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）水平宽度");
                parameters.registeR("package_depth", &_package_depth, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）垂直深度");

                parameters.registeR("package_layers", &_package_layers, make_pair(1, 10), {1, 2}, "包裹堆叠的层数");
                parameters.registeR("package_gap", &_package_gap, make_pair(1, STORAGE_MAX), "包裹之间的距离");

                parameters.registeR("stack_depth" , &_stack_depth, make_pair(1, 50), "包裹的（对拣货通道）垂直层数");

                parameters.registeR("aisle_width", &_aisle_width, make_pair(1, STORAGE_MAX), "包裹之间，拣货通道的宽度");
                parameters.registeR("moving_path_width", &_moving_path_width, make_pair(1, STORAGE_MAX), "主通道的宽度");
                parameters.registeR("shortcut_width", &_shortcut_width, make_pair(1, STORAGE_MAX), "过道的宽度");

            }

            virtual void extractParameters() override;
            virtual void generateResult(const LayoutResult &plan, pair<PackageModel, SkeletonModel> *result) const override;

            virtual ~Stack (){}
    };
}
