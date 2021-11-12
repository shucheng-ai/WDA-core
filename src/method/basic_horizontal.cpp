#include "basic.h"

namespace layout::method{

    namespace horizontal{
        struct Cell{
            int value, prev, type;
        };

        inline bool myAnd(bool x, bool y){
            return x && y;
        }
    }

    using horizontal::Cell;

    int Basic::validUnit(int gleft, int gright, int cleft, int cright, const vector<VertiResult> &vresult, int pipe){
        if (!vresult.size()) return 1;
        if (cleft < hori_acc[pipe]) return hori_last[pipe];
        int bins = 0, stability_penalty = 0; col->resetAcc(0);
        for (const auto &v : vresult){
            const int bottom = v.bottom - model->upright_width, top = v.top + model->upright_width;
            switch (v.type){
                case storage::UPRIGHT :
                    if (!col->stabilityIgnore(cleft, cright, v.bottom, v.top) && col->anyColision(cleft, cright, v.bottom, v.top))
                        stability_penalty += penalty_for_collapsed_upright;
                    break;
                case storage::PACKAGE :
                case storage::UNDERPASS_PACKAGE :
                    //TODO different_underpass_layers
                    if (!col->obstacleColision(gleft, gright, bottom, top) && !col->guardColision(cleft, cright, bottom, top)){
                        bins += v.type == storage::PACKAGE ? model->layers :
                            model->underpass_layers_for_shortcut | model->underpass_layers_for_moving_path;
                        break;
                    }
                    if (!col->stabilityIgnore(cleft, cright, bottom, top) && !validBeam(cleft, cright, bottom, top))
                        stability_penalty += penalty_for_occupied_beam;
                    break;
                case storage::EMPTY :
                    break;
                default :
                    raiseError(UnknownError, to_string(v.type));
            }
        }
        hori_last[pipe] = bins * ratio_for_bins - stability_penalty;
        hori_acc[pipe] = cleft + col->accNext();
        return bins * ratio_for_bins - stability_penalty;
    }

    void Basic::horizontalDP (const vector<VertiResult> &vresult, vector<HoriResult>* const hresult){
        const int N = size[0] + 5;
        vector<Cell> f(N), g(N); Cell tg = {0, 0, 0};
        int result_position = 0; Cell *result_iterator = &f[0];
        int left_gap = model->storage_gap / 2, right_gap = model->storage_gap - left_gap;

        hori_acc.assign(model->depth * 2, 0); hori_last.resize(model->depth + model->depth);

        for (int i = model->passage_width; i <= size[0]; i++){
            if (con->horiSuf(i - model->passage_width, i)){
                //RIGHTWARD
                int destination = i, right = i - model->passage_width, left = right - model->storage_depth;
                for (int j = 1, sum = 0; j <= model->depth && left >= right_gap; ++j, left = (right = left - model->storage_gap) - model->storage_depth){
                    int position = left - right_gap;
                    if (con->horiNec(left, right)) break;
                    sum += validUnit(left, destination, left, right, vresult, j - 1);
                    if (f[i].value < g[position].value + sum)
                        f[i] = Cell{g[position].value + sum, position, RIGHTWARD * j};
                }
                //update result 
                if (f[i].value > result_iterator->value){
                    result_position = i; result_iterator = &f[i];
                }
                if (g[i].value > result_iterator->value){
                    result_position = i; result_iterator = &g[i];
                }

                if (f[i - 1].value > f[i].value && con->horiSuf(i - 1, i)) f[i] = f[i - 1];
                //LEFTWARD
                destination = i - model->passage_width; left = i; right = left + model->storage_depth;
                for (int j = 1, sum = 0; j <= model->depth && right <= size[0] - left_gap; ++j, right = (left = right + model->storage_gap) + model->storage_depth){
                    int position = right + left_gap;
                    if (con->horiNec(left, right)) break;
                    sum += validUnit(destination, right, left, right, vresult, j + model->depth - 1);
                    if (g[position].value < f[i].value + sum){
                        g[position] = Cell{f[i].value + sum, i, LEFTWARD * j};
                    }
                }
            }
            if (con->horiNec(i - 1, i))
                tg = Cell{0, 0, 0};
            else{
                if (tg.value > g[i].value) g[i] = tg;
                if (g[i].value - penalty_for_large_storage_gap >= tg.value){
                    tg = g[i]; tg.value -= penalty_for_large_storage_gap;
                }
            }
        }

        //generate result
        auto next = [&]{
            result_position = result_iterator->prev;
            result_iterator = result_iterator->type * LEFTWARD > 0 ? &f[result_position] : &g[result_position]; //the previous one
        };
#ifdef KEYPOINT
        LOG(INFO) << result_iterator->value;
#endif
        hresult->clear();
        for (; result_position && result_iterator->type; next()){
            int left = result_iterator->prev, right = left + (model->storage_gap + model->storage_depth) * std::abs(result_iterator->type);
            if (result_iterator->type * RIGHTWARD > 0){
                left += right_gap; right -= left_gap;
            }
            else
                right -= model->storage_gap;
            hresult->emplace_back(left, right, result_iterator->type * RIGHTWARD > 0 ? RIGHTWARD : LEFTWARD);
        }
        reverse(hresult->begin(), hresult->end());
#ifdef KEYPOINT
        LOG(INFO) << "LEFTWARD : " << LEFTWARD << "; RIGHTWARD : " << RIGHTWARD;
        for (auto &h : *hresult)
            LOG(INFO) << h.left << " " << h.right << " " << h.direction;
#endif
    }
}
