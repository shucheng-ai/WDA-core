#include "basic.h"
#include "boost/multi_array.hpp"

namespace layout::method{

    namespace vertical{
        struct Cell{
            int value = numeric_limits<int>::min() / 2, prev, status;
        };
    }

    using vertical::Cell;

    pi Basic::validUnit(int bottom, int top, const vector<HoriResult> &hresult){
        if (bottom < verti_acc) return verti_last; 
        int stacks = 0, stability_penalty = 0; col->resetAcc(1);
        for (const auto &h : hresult)
            for (int i = h.left; i + model->storage_depth <= h.right; i += model->storage_depth + model->storage_gap){
                int cleft = i, cright = i + model->storage_depth;
                int gleft = h.direction == RIGHTWARD ? cleft : h.left - model->passage_width;
                int gright = h.direction == RIGHTWARD ? h.right + model->passage_width : cright;

                if (!col->obstacleColision(gleft, gright, bottom, top) && !col->guardColision(cleft, cright, bottom, top)){
                    ++stacks; continue;
                }

                if (!col->stabilityIgnore(cleft, cright, bottom, top) && !validBeam(cleft, cright, bottom, top)){
                    stability_penalty += penalty_for_occupied_beam;
                }
            }
        verti_last = pi(stacks, stability_penalty);
        verti_acc = bottom + col->accNext();
        return verti_last;
    }

    int Basic::countInvalidUpright(int bottom, int top, const vector<HoriResult> &hresult){
        if (bottom < upright_acc) return upright_last;
        int sum = 0; col->resetAcc(1);
        for (auto &h : hresult)
            for (int i = h.left; i + model->storage_depth <= h.right; i += model->storage_depth + model->storage_gap){
                int cleft = i, cright = i + model->storage_depth;
                sum += !col->stabilityIgnore(cleft, cright, bottom, top) && col->anyColision(cleft, cright, bottom, top);
            }
        upright_last = sum; upright_acc = bottom + col->accNext();
        return sum;
    }

    void Basic::verticalDP (const vector<HoriResult> &hresult, vector<VertiResult>* const vresult){
        penalty_for_liberal_storage = web_mapping[_penalty_for_liberal_storage] * model->layers * hresult.size() / 4;
        penalty_for_liberal_endstorage = web_mapping[_penalty_for_liberal_endstorage] * model->layers * hresult.size() / 4;
        penalty_for_underpass_endstorage = web_mapping[_penalty_for_underpass_endstorage] * model->layers * hresult.size() / 4;
        penalty_for_irregular_upright = web_mapping[_penalty_for_irregular_upright] * model->layers / 4;

        const int N = size[1] + 5, S = model->max_pallet_per_face + 4;
        boost::multi_array<Cell, 2> f(boost::extents[N][S]);
        const int UPRIGHT = model->max_pallet_per_face + 1, MOVING_PATH = UPRIGHT + 1, SHORTCUT = MOVING_PATH + 1, INIT = SHORTCUT + 1;
        int result_position = 0, result_status = UPRIGHT;
        unique_ptr<Cell> result(new Cell()); Cell *result_iterator = result.get();

        auto update = [&](int i, Cell const &source, int penalty = 0){
            if (source.value - penalty > result_iterator->value){
                result_position = i; *result_iterator = source;
                result_iterator->value -= penalty;
            }
        };

        //vertical acceleration
        verti_acc = 0; verti_offline.resize(size[1]);
        //off-line precalculate
        for (int i = model->upright_width; i + model->package_width + model->upright_width <= size[1]; ++i)
            verti_offline[i - model->upright_width] = validUnit(i - model->upright_width, i + model->package_width + model->upright_width, hresult);
        auto accValidUnit = [&](int bottom, int top, int layers){
            return verti_offline[bottom].first * layers * ratio_for_bins - verti_offline[bottom].second;
        };

        //upright acceleration
        upright_acc = 0;
        //dp
        for (int i = model->upright_width; i <= size[1]; ++i){
            //all liberal storage at gap except upright
            if (!con->vertiNec(i - 1, i)){
                for (int j = 0; j <= model->max_pallet_per_face; ++j)
                    if (f[i - 1][j].value > f[i][j].value) f[i][j] = f[i - 1][j];
            }
            if (con->vertiSuf(i - 1, i)){
                if (f[i - 1][MOVING_PATH].value > f[i][MOVING_PATH].value && !con->vertiNecForShortcut(i - 1, i)) 
                    f[i][MOVING_PATH] = f[i - 1][MOVING_PATH];
                if (f[i - 1][SHORTCUT].value > f[i][SHORTCUT].value && !con->vertiNecForMovingPath(i - 1, i))
                    f[i][SHORTCUT] = f[i - 1][SHORTCUT];
            }

            int top = i, bottom = top - model->upright_width;
            if (!con->vertiNec(bottom, top) && !extra->vertiPath(bottom, top)){

                //standard liberal at upright
                Cell &base = f[top][UPRIGHT]; bool flag = 0;
                for (int j = model->max_pallet_per_face; j >= 0; --j)
                    if (f[bottom][j].value > base.value) base = Cell{f[bottom][j].value, bottom, j}; //upright 1

                //update result with liberal storage at the endpoint
                update(top, base, penalty_for_liberal_endstorage);

                base.value -= countInvalidUpright(bottom, top, hresult) * penalty_for_collapsed_upright;
                if (extra->vertiStorage(bottom, top)) base.value -= penalty_for_irregular_upright;

                //the beginning of liberal storage
                int value = base.value - penalty_for_liberal_storage;
                if (f[top][0].value <= value) f[top][0] = Cell{value, top, UPRIGHT};

                //the beginning of moving_path or shortcut
                if (f[top][MOVING_PATH].value <= value) f[top][MOVING_PATH] = Cell{value, top, UPRIGHT};
                if (f[top][SHORTCUT].value <= value) f[top][SHORTCUT] = Cell{value, top, UPRIGHT};

                //init
                if (base.value < 0) base = Cell{0, top, INIT}; //begin with standard storage
                if (f[top][0].value <= -penalty_for_liberal_endstorage - penalty_for_liberal_storage) f[top][0] = Cell{-penalty_for_liberal_endstorage - penalty_for_liberal_storage, top, INIT}; //begin with liberal storage
                if (f[top][MOVING_PATH].value <= -penalty_for_underpass_endstorage - penalty_for_liberal_storage) f[top][MOVING_PATH] = Cell{-penalty_for_underpass_endstorage - penalty_for_liberal_storage, top, INIT}; //begin with underpass for moving_path
                if (f[top][SHORTCUT].value <= -penalty_for_underpass_endstorage - penalty_for_liberal_storage) f[top][SHORTCUT] = Cell{-penalty_for_underpass_endstorage - penalty_for_liberal_storage, top, INIT}; //begin with underpass for shortcut

                //standard regular storage
                top = i;
                if (model->pallet_per_face * model->package_width + top + model->upright_width <= size[1]){
                    int sum = 0; bool flag = 0;
                    for (int j = 0; j < model->pallet_per_face; ++j){
                        bottom = top; top = bottom + model->package_width;
                        if (con->vertiNec(bottom - model->upright_width, top + model->upright_width) || extra->vertiPath(bottom - model->upright_width, top + model->upright_width)){
                            flag = 1; break;
                        }
                        sum += accValidUnit(bottom - model->upright_width, top + model->upright_width, model->layers);
                    }
                    if (!flag){
                        Cell &SRtarget = f[top + model->upright_width][UPRIGHT];
                        Cell source = Cell{base.value + sum, i, UPRIGHT}; //a group of packages 2
                        if (SRtarget.value < source.value) SRtarget = source;
                        //update result with regular storage at the endpoint
                        update(top + model->upright_width, source);
                    }
                }

                //standard liberal storage at package
                bottom = i; top = bottom + model->package_width;
                if (top + model->upright_width <= size[1] && !con->vertiNec(bottom - model->upright_width, top + model->upright_width) && !extra->vertiPath(bottom - model->upright_width, top + model->upright_width)){
                    int value = accValidUnit(bottom - model->upright_width, top + model->upright_width, model->layers);
                    for (int j = 0; j < model->max_pallet_per_face; ++j){
                        Cell &SLtarget = f[top][j + 1];
                        if (SLtarget.value < f[i][j].value + value) SLtarget = Cell{f[i][j].value + value, i, j}; //single package 3
                    }
                }
            }

            //moving path : regular but adjustable storage for underpass
            if (i + model->moving_path_width + model->upright_width <= size[1] && con->vertiSuf(i, i + model->moving_path_width) && !con->vertiNecForShortcut(i, i + model->moving_path_width))
                if (model->underpass_layers_for_moving_path){
                    top = i + std::max(model->moving_path_width - model->underpass_pallet_per_face * model->package_width, 0);
                    for (int j = 1, sum = 0; j <= model->underpass_pallet_per_face; ++j){
                        bottom = top; top = bottom + model->package_width; 
                        sum += accValidUnit(bottom - model->upright_width, top + model->upright_width, model->underpass_layers_for_moving_path);
                        if (top + model->upright_width > size[1]) break;
                        if (top - i < model->moving_path_width) continue; //slightly different from the reality
                        if (!con->vertiSuf(bottom, top) || con->vertiNecForShortcut(bottom, top)) break;
                        if (con->vertiNec(top, top + model->upright_width)) continue;
                        Cell &URtarget = f[top + model->upright_width][UPRIGHT];
                        Cell source = Cell{f[i][MOVING_PATH].value + sum, i, MOVING_PATH}; //a group of packages 4
                        if (URtarget.value < source.value) URtarget = source;
                        //update result with moving path at endpoint
                        update(top + model->upright_width, source, penalty_for_underpass_endstorage);
                    }
                }
                else{
                    Cell &target = f[i + model->moving_path_width + model->upright_width][UPRIGHT];
                    if (target.value < f[i][MOVING_PATH].value) target = Cell{f[i][MOVING_PATH].value, i, MOVING_PATH}; //empty moving path 4
                }

            //shortcut
            if (i + model->shortcut_width + model->upright_width <= size[1] && con->vertiSuf(i, i + model->shortcut_width) && !con->vertiNecForMovingPath(i, i + model->shortcut_width))
                if (model->underpass_layers_for_shortcut){
                    top = i + std::max(model->shortcut_width - model->underpass_pallet_per_face * model->package_width, 0);
                    for (int j = 1, sum = 0; j <= model->underpass_pallet_per_face; ++j){
                        bottom = top; top = bottom + model->package_width;
                        sum += accValidUnit(bottom - model->upright_width, top + model->upright_width, model->underpass_layers_for_shortcut);
                        if (top + model->upright_width > size[1]) break;
                        if (top - i < model->shortcut_width) continue;
                        if (!con->vertiSuf(bottom, top) || con->vertiNecForMovingPath(bottom, top)) break;
                        if (con->vertiNec(top, top + model->upright_width)) continue;
                        Cell &URtarget = f[top + model->upright_width][UPRIGHT];
                        Cell source = Cell{f[i][SHORTCUT].value + sum, i, SHORTCUT}; //a group of packages 5
                        if (URtarget.value < source.value) URtarget = source;
                        //update result with shortcut at endpoint
                        update(top + model->upright_width, source, penalty_for_underpass_endstorage);
                    }
                }
                else{
                    Cell &target = f[i + model->shortcut_width + model->upright_width][UPRIGHT];
                    if (target.value < f[i][SHORTCUT].value) target = Cell{f[i][SHORTCUT].value, i, SHORTCUT}; //empty shortcut 5
                }
        }

        //generate vresult
        if (result_iterator->value <= 0) raiseError(MethodError, "invalid vertical arrangement");
        vresult->clear();
        auto next = [&]{
            result_position = result_iterator->prev;
            result_status = result_iterator->status;
            result_iterator = &f[result_position][result_status];
        };
#ifdef KEYPOINT
        LOG(INFO) << result_iterator->value;
#endif
        for (; result_position; next()){
            //place a SR storage
            if (result_status == UPRIGHT && result_iterator->status == UPRIGHT){
                vresult->emplace_back(result_position - model->upright_width, result_position, storage::UPRIGHT);
                for (int i = result_position - model->upright_width; i > result_iterator->prev; i -= model->package_width)
                    vresult->emplace_back(i - model->package_width, i, storage::PACKAGE);
                continue;
            }
            //place a single package of SL storage
            if (result_status < UPRIGHT && result_status == result_iterator->status + 1){
                vresult->emplace_back(result_iterator->prev, result_iterator->prev + model->package_width, storage::PACKAGE);
                continue;
            }
            //place a moving path
            if (result_status == UPRIGHT && result_iterator->status == MOVING_PATH){
                vresult->emplace_back(result_position - model->upright_width, result_position, storage::UPRIGHT);
                if (model->underpass_layers_for_moving_path)
                    for (int i = result_position - model->upright_width, j = 0; i > result_iterator->prev && j < model->underpass_pallet_per_face; i -= model->package_width, ++j)
                        vresult->emplace_back(i - model->package_width, i, storage::UNDERPASS_PACKAGE);
                else
                    vresult->emplace_back(result_iterator->prev, result_position, storage::EMPTY);
                continue;
            }
            //place a shortcut
            if (result_status == UPRIGHT && result_iterator->status == SHORTCUT){
                vresult->emplace_back(result_position - model->upright_width, result_position, storage::UPRIGHT);
                if (model->underpass_layers_for_shortcut)
                    for (int i = result_position - model->upright_width, j = 0; i > result_iterator->prev && j < model->underpass_pallet_per_face; i -= model->package_width, ++j)
                        vresult->emplace_back(i - model->package_width, i, storage::UNDERPASS_PACKAGE);
                else
                    vresult->emplace_back(result_iterator->prev, result_position, storage::EMPTY);
                continue;
            }
            //initial upright
            if (result_iterator->status == INIT){
                vresult->emplace_back(result_iterator->prev - model->upright_width, result_iterator->prev, storage::UPRIGHT);
                break;
            }

            //place a upright for SL storage
            if (result_status == UPRIGHT){
                vresult->emplace_back(result_position - model->upright_width, result_position, storage::UPRIGHT);
                continue;
            }

            //liberal package to begining upright
            if (result_iterator->status == UPRIGHT && (result_status > UPRIGHT || !result_status)) continue;

            raiseError(UnknownError, to_string(result_position) + " " + to_string(result_iterator->prev) + " " + to_string(result_status) + " " + to_string(result_iterator->status));
        }

        reverse(vresult->begin(), vresult->end());
#ifdef KEYPOINT
        for (auto &v : *vresult)
            LOG(INFO) << v.bottom << " " << v.top << " " << v.type;
#endif
    }

}
