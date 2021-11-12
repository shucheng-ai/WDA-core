#include "shelf.h"
#include "method/method.h"

#define for_shelf_depth(x) for (int i = 0, x = row.left + edge_clearance; i < _shelf_depth; ++i, x += _package_depth + _package_gap)

namespace layout::storage{

    namespace{
        StorageModel::Registrar _1("shelf", []{return new Shelf();});
    }

    void Shelf::extractParameters (){
        layers = 0; info = "shelf";
        for_shelf_height(h) layers++;
        if (!layers) raiseError(StorageError, "Derived layers is smaller or equal to 0");

        storage_depth = _storage_depth;
        if (_package_depth * _shelf_depth + _package_gap * (_shelf_depth - 1) > _storage_depth) raiseError(StorageError, "package depth should not larger than storage depth for Shelf");
        if (_storage_depth < 2 * _upright_depth) raiseError(StorageError, "storage depth shold not smaller than 2 times of upright_depth");

        depth = 1; beam_depth = _upright_depth;
        passage_width = _aisle_width; storage_gap = _storage_gap;

        int derived_storage_wdith = _pallet_per_face * _package_width + _package_gap * _pallet_per_face; 
        if (derived_storage_wdith != _storage_width) raiseError(StorageError, "the storage width is not equal to the derved one which should be : " + to_string(derived_storage_wdith));

        upright_width = _upright_width; //与apr不同
        package_width = _package_width + _package_gap;
        pallet_per_face = _pallet_per_face; max_pallet_per_face = _max_pallet_per_face;
        underpass_pallet_per_face = 0;

        moving_path_width = _moving_path_width; shortcut_width = _shortcut_width;
        underpass_layers_for_shortcut = underpass_layers_for_moving_path = 0;

        allow_involved_column = 0;
    }

    void Shelf::generateResult (const LayoutResult &plan, pair<PackageModel, SkeletonModel>* result) const{
        auto &packages = result->first; int edge_clearance = (_storage_depth - _package_depth * _shelf_depth - _package_gap * (_shelf_depth - 1)) / 2; 
        //PackageModel
        for (auto const &row : plan.rows){
            if (packages.empty())
                packages.emplace_back();
            else
                if (row.direction == RIGHTWARD) packages.emplace_back();
            vector<int> hori_order;
            for_shelf_depth(x) hori_order.push_back(x);
            if (row.direction != RIGHTWARD) reverse(hori_order.begin(), hori_order.end());
            auto &temp = packages.back(); temp.resize(plan.vpoints.size() + 1);
            auto cur = temp.begin(); auto split_point = plan.vpoints.cbegin();
            for (auto const &v : row){
                if (split_point != plan.vpoints.cend() && (v.bottom + v.top) / 2> split_point->second){
                    ++cur; ++split_point;
                }
                if (v.type == PACKAGE)
                    for_shelf_height(h)
                        for (auto x : hori_order)
                            for (int i = 0; i < _shelf_layers; ++i)
                                cur->emplace_back(Box3D{{x, v.bottom + _package_gap / 2, h + i * _package_height}, {x + _package_depth, v.top - (_package_gap + 1) / 2, h + (i + 1) * _package_height}});
            }
        }

        //SkeletonModel
        auto &skeleton = result->second;
        skeleton.emplace_back(Item::create("upright"));
        skeleton.emplace_back(Item::create("beam"));
        unique_ptr<Item> &upright = skeleton[0]; unique_ptr<Item> &beam = skeleton[1];
        for (auto const &row : plan.rows){
            int left = row.left, right = row.right;
            int color = EMPTY, last = 0;
            for (auto const &it : row){
                switch (it.type){
                    case PACKAGE :
                        if (color == UPRIGHT) color = it.type;
                        if (color != it.type) raiseError(UnknownError, R"pddoc(
                            Patience is the foundation of eternal peace. Make anger your enemy.
                            Harm comes to those who know only victory and do not know defeat.
                            Find fault with yourself and not with others.
                            It is in falling short of your own goals that you will surpass those who exceed theirs.)pddoc");
                            break;
                    case UPRIGHT :
                        if (color == PACKAGE)
                            for_shelf_height(h)
                                beam->insert1(left, right, last, it.top, h, h + _beam_height, _upright_depth);    
                        if (color == UPRIGHT)
                            for_shelf_height(h)
                                beam->insert1(left, right, last, it.top, h, h + _beam_height, _upright_depth);
                        upright->insert1(left, right, it.bottom, it.top, 0, _upright_height, _upright_depth);
                        color = UPRIGHT; last = it.bottom;
                        break;
                    case EMPTY :
                        color = EMPTY;
                    default :
                        break;
                }
            }
        }
    }

}
