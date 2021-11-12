#include "apr.h"
#include "method/method.h"

namespace layout::storage{

    namespace{
        StorageModel::Registrar _1("apr", []{return new Apr();});
    }

    void Apr::extractParameters (){
        layers = 0; info = "apr";
        for_apr_height(h) ++layers;
        if (!layers) raiseError(StorageError, "Derived layers is smaller or equal to 0");

        storage_depth = _package_depth;

        if (_package_depth < _storage_depth) raiseError(StorageError, "package depth should not smaller than storage depth for APR");
        if (_storage_depth < 2 * _upright_depth) raiseError(StorageError, "storage depth shold not smaller than 2 times of upright_depth");

        depth = _apr_depth;
        beam_depth = _upright_depth + ((_package_depth - _storage_depth + 1) / 2);
        passage_width = _aisle_width - _package_depth + _storage_depth; //passage_width 为算法的拣货通道宽度 _aisle_width 为实际货架之间的拣货通道宽
        storage_gap = _storage_gap - _package_depth + _storage_depth; //_storage_gap 为货架之间的gap

        if (passage_width < 0 || storage_gap < 0) raiseError(StorageError, "package depth is too large to keep a reasonable passage or storage gap");

        int derived_storage_wdith = _pallet_per_face * _package_width + _package_gap * (_pallet_per_face + 1); 
        //just verify it
        if (derived_storage_wdith != _storage_width) raiseError(StorageError, "the storage width is not equal to the derved one which should be : " + to_string(derived_storage_wdith));

        upright_width = _upright_width + _package_gap; //与shelf不同
        package_width = _package_width + _package_gap;
        pallet_per_face = _pallet_per_face; max_pallet_per_face = _max_pallet_per_face;
        underpass_pallet_per_face = _underpass_pallet_per_face;

        int underpass_layers = 0;
        for_apr_height(h)
            if (_underpass_height + _beam_height <= h) ++underpass_layers;
        if (!underpass_layers && (_underpass_for_shortcut || _underpass_for_shortcut))
            raiseError(StorageError, "underpass is asked to be supportted, but the layers of underpass is smaller or equal to 0");

        int derived_underpass_width = underpass_pallet_per_face * package_width; //sightly different with human logic
        moving_path_width = _moving_path_width - _package_gap; shortcut_width = _shortcut_width - _package_gap;
        if (_underpass_for_moving_path)
            underpass_layers_for_moving_path = underpass_layers;
        else underpass_layers_for_moving_path = 0;
        if (_underpass_for_shortcut)
            underpass_layers_for_shortcut = underpass_layers;
        else underpass_layers_for_shortcut = 0;

        allow_involved_column = _allow_involved_columns;
    }

    namespace{

        bool cmpL (pyBox3D const &x, pyBox3D const &y){
            if (x.first[1] != y.first[1]) return x.first[1] < y.first[1];
            if (x.first[2] != y.first[2]) return x.first[2] < y.first[2];
            return x.first[0] > y.first[0];
        }

        bool cmpR (pyBox3D const &x, pyBox3D const &y){
            if (x.first[1] != y.first[1]) return x.first[1] < y.first[1];
            if (x.first[2] != y.first[2]) return x.first[2] < y.first[2];
            return x.first[0] < y.first[0];
        }
    }

    void Apr::generateResult (const LayoutResult &plan, pair<PackageModel, SkeletonModel>* result) const{
        int last_direction = LEFTWARD; auto &packages = result->first;
        //PackageModel
        for (auto left = plan.rows.cbegin(); left != plan.rows.cend();){
            auto right = left; 
            RecursiveBox3D temp; temp.resize(plan.vpoints.size() + 1);
            for (; right != plan.rows.cend() && right->direction == left->direction; ++right){
                auto cur = temp.begin(); auto split_point = plan.vpoints.cbegin();
                for (auto const &v : *right){
                    if (split_point != plan.vpoints.cend() && (v.bottom + v.top) / 2 > split_point->second){
                        ++cur; ++split_point;
                    }
                    switch (v.type){
                        case PACKAGE :
                            for_apr_height(h)
                                cur->emplace_back(Box3D{{right->left, v.bottom + _package_gap / 2, h}, {right->right, v.top - (_package_gap + 1) / 2, h + _package_height}});
                            break;
                        case UNDERPASS_PACKAGE :
                            for_apr_height(h)
                                if (_underpass_height + _beam_height <= h) cur->emplace_back(Box3D{{right->left, v.bottom + _package_gap / 2, h}, {right->right, v.top - (_package_gap + 1) / 2, h + _package_height}});
                            break;
                        default :
                            break;
                    }
                }
            }
            if (left->direction != LEFTWARD || last_direction != RIGHTWARD) packages.emplace_back();
            auto &cur = packages.back(); cur.resize(plan.vpoints.size() + 1);
            for (int i = 0; i <= plan.vpoints.size(); ++i){
                auto &x = temp[i]; auto &y = cur[i];
                if (x.empty()) continue;
                sort(x.begin(), x.end(), left->direction == LEFTWARD ? cmpL : cmpR);
                Box3D last;
                for (auto &box : x){
                    if (box.first[1] != last.first[1] || box.first[2] != last.first[2]) y.emplace_back();
                    y.back().emplace_back(box);
                    last = box;;
                }
            }
            last_direction = left->direction; left = right;
        }
        //SkeletonModel
        auto &skeleton = result->second;
        skeleton.emplace_back(Item::create("upright"));
        skeleton.emplace_back(Item::create("beam"));
        unique_ptr<Item> &upright = skeleton[0]; unique_ptr<Item> &beam = skeleton[1];
        for (auto const &row : plan.rows){
            int left = row.left + (_package_depth - _storage_depth + 1) / 2, right = row.right - (_package_depth - _storage_depth) / 2;
            int color = EMPTY, last = 0;
            for (auto const &it : row){
                switch (it.type){
                    case PACKAGE :
                    case UNDERPASS_PACKAGE :
                        if (color == UPRIGHT) color = it.type;
                        if (color != it.type) raiseError(UnknownError, R"pddoc(
                            Patience is the foundation of eternal peace. Make anger your enemy.
                            Harm comes to those who know only victory and do not know defeat.
                            Find fault with yourself and not with others.
                            It is in falling short of your own goals that you will surpass those who exceed theirs.)pddoc");
                        break;
                    case UPRIGHT :
                        if (color == PACKAGE)
                            for_apr_height(h)
                                beam->insert1(left, right, last, it.top - (_package_gap + 1) / 2, h, h + _beam_height, _upright_depth);    
                        if (color == UNDERPASS_PACKAGE)
                            for_apr_height(h)
                                if (_underpass_height + _beam_height <= h) beam->insert1(left, right, last, it.top - (_package_gap + 1) / 2, h, h + _beam_height, _upright_depth);
                        if (color == UPRIGHT)
                            for_apr_height(h)
                                beam->insert1(left, right, last, it.top, h, h + _beam_height, _upright_depth);
                        color = UPRIGHT; last = it.bottom + _package_gap / 2;
                        upright->insert1(left, right, last, it.top - (_package_gap + 1) / 2, 0, _upright_height, _upright_depth);
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

