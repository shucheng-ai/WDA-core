#pragma once
#include "storage.h"

#define for_apr_height(i)  for (int i = _floor_clearance ? _floor_clearance + _beam_height : 0; \
        i <= _upright_height - _upright_top_reserved_end; \
        i += _package_height + _upward_clearance + _beam_height)

namespace layout::storage{
    
    class Apr : public StorageModel{
        protected : 
            int _package_height = 1500;
            int _package_width = 1000;
            int _package_depth = 1200;
            int _package_gap = 100;
            int _aisle_width = 3000;
            int _moving_path_width = 3500;
            int _shortcut_width = 3500;

            int _upward_clearance = 150;
            int _apr_depth = 1;

            int _upright_height = 7500;
            int _upright_width = 90;
            int _upright_depth = 90;
            int _upright_top_reserved_end = 300;
            int _allow_involved_columns = 1;
            int _storage_depth = 1000;
            int _storage_gap = 300;

            int _storage_width = 2300;
            int _underpass_pallet_per_face = 2;
            int _pallet_per_face = 2;
            int _max_pallet_per_face = 3;

            int _beam_height = 120;
            int _floor_clearance = 0;

            int _underpass_height = 3500;
            int _underpass_for_moving_path = 1;
            int _underpass_for_shortcut = 1;

        public :

            Apr (){

                parameters.registeR("allow_involvded_column", &_allow_involved_columns, make_pair(0 , 1) , {0 , 1}, "是否允许用货架包裹柱子");

                parameters.registeR("package_depth", &_package_depth, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）垂直深度");
                parameters.registeR("package_width", &_package_width, make_pair(1, STORAGE_MAX), "包裹的（对拣货通道）水平宽度");
                parameters.registeR("package_height", &_package_height, make_pair(1, STORAGE_MAX), "包裹的高度");

                parameters.registeR("upward_clearance", &_upward_clearance, make_pair(1, STORAGE_MAX), "包裹上方留空");
                parameters.registeR("package_gap", &_package_gap, make_pair(1, STORAGE_MAX), "包裹之间距离");
                
                parameters.registeR("apr_depth", &_apr_depth, make_pair(1,2), {1, 2}, "货架的（对拣货通道）的垂直层数");
                parameters.registeR("pallet_per_face", &_pallet_per_face, make_pair(1, 10), "横梁上的标准水平包裹数");
                parameters.registeR("max_pallet_per_face", &_max_pallet_per_face, make_pair(1, 10), "货架上的最大水平包裹数");

                parameters.registeR("storage_depth", &_storage_depth, make_pair(1, STORAGE_MAX), "货架的（对拣货通道）垂直深度");
                parameters.registeR("storage_width", &_storage_width, make_pair(1, STORAGE_MAX), "货架的（对拣货通道）水平宽度");
                parameters.registeR("storage_gap", &_storage_gap, make_pair(1, STORAGE_MAX), "货架背靠背的留空");

                parameters.registeR("aisle_width", &_aisle_width, make_pair(1, STORAGE_MAX), "货架间，拣货通道的宽度");
                parameters.registeR("moving_path_width", &_moving_path_width, make_pair(1, STORAGE_MAX), "主通道的宽度");
                parameters.registeR("shortcut_width", &_shortcut_width, make_pair(1, STORAGE_MAX), "过道的宽度");
                
                parameters.registeR("underpass_height", &_underpass_height, make_pair(1, STORAGE_MAX), "underpass的通行高度");
                parameters.registeR("underpass_for_moving_path", &_underpass_for_moving_path, make_pair(0, 1), {0, 1}, "是否允许对主通道搭建underpass");
                parameters.registeR("underpass_for_shortcut", &_underpass_for_shortcut, make_pair(0, 1), {0, 1}, "是否允许对过道搭建underpass");
                parameters.registeR("underpass_pallet_per_face", &_underpass_pallet_per_face, make_pair(1, 10), "underpass的横梁上的标准水平包裹数");

                parameters.registeR("beam_height", &_beam_height, make_pair(1, STORAGE_MAX), "横梁的高度");
                parameters.registeR("upright_height", &_upright_height, make_pair(1, STORAGE_MAX), "立柱的高度");
                parameters.registeR("upright_depth", &_upright_depth, make_pair(1, STORAGE_MAX), "立柱的（对拣货通道）垂直深度");
                parameters.registeR("upright_width", &_upright_width, make_pair(1, STORAGE_MAX), "立柱的（对拣货通道）的水平宽度");
                parameters.registeR("upright_top_reserved_end", &_upright_top_reserved_end, make_pair(0, STORAGE_MAX), "立柱上方突出高度");

                parameters.registeR("floor_clearance", &_floor_clearance, make_pair(0, STORAGE_MAX), "最底层留空的高度");
            }

            virtual void extractParameters() override;
            virtual void generateResult(const LayoutResult &plan, pair<PackageModel, SkeletonModel> *result) const override;

            virtual ~Apr() {}
    };

}
