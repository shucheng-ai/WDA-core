#pragma once
#include "storage.h"

#define for_shelf_height(i) for (int i = _floor_clearance ? _floor_clearance + _beam_height : 0; \
                i <= _upright_height; \
                i += _package_height * _shelf_layers + _upward_clearance + _beam_height)

namespace layout::storage{

    class Shelf : public StorageModel{
        protected :
            int _package_height = 400;
            int _package_width = 400;
            int _package_depth = 600;
            int _package_gap = 50;
            int _aisle_width = 900;
            int _moving_path_width = 2000;
            int _shortcut_width = 2000;

            int _upward_clearance = 20;
            int _shelf_depth = 1;
            int _shelf_layers = 1;

            int _upright_height = 2000;
            int _upright_width = 55;
            int _upright_depth = 30;
            int _storage_depth = 600;
            int _storage_gap = 20;

            int _storage_width = 1350;
            int _pallet_per_face = 3;
            int _max_pallet_per_face = 4;

            int _beam_height = 60;
            int _floor_clearance = 40;

        public :

            Shelf (){

                parameters.registeR("package_height", &_package_height, make_pair(1, STORAGE_MAX), "包裹的高度");
                parameters.registeR("package_width", &_package_width, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）水平宽度");
                parameters.registeR("package_depth", &_package_depth, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）垂直深度");

                parameters.registeR("upward_clearance", &_upward_clearance, make_pair(1, STORAGE_MAX), "包裹上方留空");
                parameters.registeR("package_gap", &_package_gap, make_pair(1, STORAGE_MAX), "包裹之间的距离");
                parameters.registeR("shelf_layers", &_shelf_layers, make_pair(1, 10), {1, 2}, "同一层包裹堆叠的层数");

                parameters.registeR("shelf_depth", &_shelf_depth, make_pair(1, 10), {1, 2}, "包裹的（对拣货通道）垂直数目");
                parameters.registeR("pallet_per_face", &_pallet_per_face, make_pair(1, 10), "同一层的标准水平包裹数目");
                parameters.registeR("max_pallet_per_face", &_max_pallet_per_face, make_pair(1, 10), "同一层的最大水平包裹数");

                parameters.registeR("storage_depth", &_storage_depth, make_pair(1, STORAGE_MAX), "货架的（对拣货通道）垂直深度");
                parameters.registeR("storage_width", &_storage_width, make_pair(1, STORAGE_MAX), "货架的（对拣货通道）水平宽度");
                parameters.registeR("storage_gap", &_storage_gap, make_pair(1, STORAGE_MAX), "货架背靠背的留空");

                parameters.registeR("aisle_width", &_aisle_width, make_pair(1, STORAGE_MAX), "货架间，拣货通道的宽度");
                parameters.registeR("moving_path_width", &_moving_path_width, make_pair(1, STORAGE_MAX), "主通道的宽度");
                parameters.registeR("shortcut_width", &_shortcut_width, make_pair(1, STORAGE_MAX), "过道的宽度");

                parameters.registeR("beam_height", &_beam_height, make_pair(1, STORAGE_MAX), "横梁高度");
                parameters.registeR("upright_height", &_upright_height, make_pair(1, STORAGE_MAX), "立柱的高度");
                parameters.registeR("upright_depth", &_upright_depth, make_pair(1, STORAGE_MAX), "立柱的（对拣货通道）深度");
                parameters.registeR("upright_width", &_upright_width, make_pair(1, STORAGE_MAX), "立柱的（对拣货通道）宽度");

                parameters.registeR("floor_clearance", &_floor_clearance, make_pair(0, STORAGE_MAX), "最底层留空的高度");

            }

            virtual void extractParameters() override;
            virtual void generateResult(const LayoutResult &plan, pair<PackageModel, SkeletonModel> *result) const override;

            virtual ~Shelf () {}
    };
}
