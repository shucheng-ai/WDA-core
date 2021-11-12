#include "stack.h"
#include "method/method.h"

namespace layout::storage{

    namespace {
        StorageModel::Registrar _1("stack", []{return new Stack();});
    }

    void Stack::extractParameters (){
        layers = _package_layers; info = "stack";

        storage_depth = _package_depth; depth = _stack_depth;
        passage_width = _aisle_width; beam_depth = 0; storage_gap = _package_gap;

        upright_width = _package_gap; package_width = _package_width;
        pallet_per_face = max_pallet_per_face = 1;
        underpass_pallet_per_face = 0;

        moving_path_width = _moving_path_width; shortcut_width = _shortcut_width;
        underpass_layers_for_shortcut = underpass_layers_for_moving_path = 0;

        allow_involved_column = 0;
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

    void Stack::generateResult (const LayoutResult &plan, pair<PackageModel, SkeletonModel>* result) const{
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
                    if (v.type == PACKAGE)
                        for_stack_height(h)
                            cur->emplace_back(Box3D{{right->left, v.bottom, h}, {right->right, v.top, h + _package_height}});
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
                    last = box;
                }
            }
            last_direction = left->direction; left = right;
        }
        //SkeletonModel
    }
}

