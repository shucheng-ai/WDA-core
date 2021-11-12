#include "basic.h"

#define ifPackage(x) ((x) == storage::UNDERPASS_PACKAGE || (x) == storage::PACKAGE)

namespace layout::method{

    namespace{
        enum{
            EMPTY,
            STORAGE,
            PATH
        };

        void popUpright (int top_type, IndividualResult* const row){
            while (!row->empty() && row->back().type == storage::UPRIGHT){
                int bottom_type = row->size() > 1 ? row->at(row->size() - 2).type : storage::EMPTY;
                if (ifPackage(bottom_type)) break;
                row->pop_back();
            }
            if (top_type == storage::EMPTY && !row->empty() && row->back().type == storage::EMPTY) row->pop_back();
        }
    }

    void Basic::generateLocalResult (const vector<VertiResult> &vresult, int gleft, int gright, IndividualResult* const row){
        row->clear();
        for (const auto &v : vresult){
            int cleft = row->left, cright = row->right, bottom = v.bottom, top = v.top;
            switch (v.type){
                case storage::PACKAGE :
                case storage::UNDERPASS_PACKAGE :
                    bottom -= model->upright_width; top += model->upright_width;
                    if (!col->obstacleColision(gleft, gright, bottom, top) && !col->guardColision(cleft, cright, bottom, top)){
                        if (row->empty() || row->back().type == storage::EMPTY)
                            row->emplace_back(bottom, v.bottom, storage::UPRIGHT);
                        else
                            if (!validBeam(cleft, cright, row->back().top, bottom)){
                                if (row->back().type == storage::UPRIGHT) popUpright(storage::EMPTY, row);
                                if (!row->empty() && ifPackage(row->back().type)) row->emplace_back(row->back().top, row->back().top + model->upright_width, storage::UPRIGHT);
                                if (!row->empty())
                                    row->emplace_back(row->back().top, bottom, storage::EMPTY);
                                row->emplace_back(bottom, v.bottom, storage::UPRIGHT);
                            }
                        row->emplace_back(v.bottom, v.top, v.type);
                    }
                    break;
                case storage::UPRIGHT :
                    if (!row->empty() && row->back().type != storage::EMPTY){
                        int type = row->back().type;
                        if (validBeam(cleft, cright, row->back().top, bottom)){
                            if (col->anyColision(cleft, cright, bottom, top))
                                if (type == storage::PACKAGE || type == storage::UNDERPASS_PACKAGE)
                                    row->emplace_back(row->back().top, row->back().top + model->upright_width, storage::UPRIGHT);
                                else;
                            else
                                if (type == storage::UPRIGHT) popUpright(storage::UPRIGHT, row);
                        }
                        else{
                            switch (type){
                                case storage::UPRIGHT :
                                    popUpright(storage::EMPTY, row);
                                    break;
                                case storage::PACKAGE :
                                case storage::UNDERPASS_PACKAGE :
                                    row->emplace_back(row->back().top, row->back().top + model->upright_width, storage::UPRIGHT);
                                    break;
                                default :
                                    break;
                            }
                            if (!row->empty()) row->emplace_back(row->back().top, bottom, storage::EMPTY);
                        }
                    }
                    if (!col->anyColision(cleft, cright, bottom, top))
                        row->emplace_back(bottom, top, storage::UPRIGHT);
                    break;
                case storage::EMPTY :
                    row->emplace_back(bottom, top, storage::EMPTY);
                    break;
                default :
                    break;
            }
        }
        if (!row->empty() && row->back().type == storage::UPRIGHT) popUpright(storage::EMPTY, row);
        for (auto it = row->begin(); it != row->end(); ++it)
            if (it->type == storage::EMPTY){
                it->bottom = (it - 1)->top; it->top = (it + 1)->bottom;
            }
    }

    void Basic::generateResult (){
        result->clear();

        //generate ExtraConnectionSystem to ensure the local result is bounded by global result
        int color = EMPTY, last = 0;
        for (auto &v : vresult){
            switch (v.type){
                case storage::UPRIGHT :
                    if (color == STORAGE) extra->storage.emplace_back(last, v.bottom);
                    if (color == PATH){
                        extra->path.emplace_back(last, v.bottom);
                        result->vpoints.emplace_back(last, v.bottom);
                    }
                    last = v.top; color = EMPTY;
                    break;
                case storage::EMPTY :
                case storage::UNDERPASS_PACKAGE :
                    if (color == STORAGE) raiseError(UnknownError, "unexpected error");
                    color = PATH;
                    break;
                case storage::PACKAGE :
                    if (color == PATH) raiseError(UnknownError, "unexpected error");
                    color = STORAGE;
                    break;
                default : 
                    break;
            }
        }
        extra->update();
#ifdef KEYPOINT
        LOG(INFO) << extra->python();
#endif
        int last_direction = LEFTWARD;

        for (auto &h : hresult){
            if (last_direction != RIGHTWARD || h.direction != LEFTWARD)
                if (h.direction == LEFTWARD)
                    result->hpoints.emplace_back(h.left - model->passage_width, h.left);
                else
                    result->hpoints.emplace_back(h.right, h.right + model->passage_width);
            last_direction = h.direction;
            if (allow_individual_arrangement){
                vector<HoriResult> temp = {h};
                verticalDP(temp, &vresult);
            }
            for (int i = h.left; i + model->storage_depth <= h.right; i += model->storage_depth + model->storage_gap){
                int cleft = i, cright = i + model->storage_depth;
                int gleft = h.direction == RIGHTWARD ? cleft : h.left - model->passage_width;
                int gright = h.direction == RIGHTWARD ? h.right + model->passage_width : cright;
                result->rows.emplace_back(cleft, cright, h.direction);
                generateLocalResult(vresult, gleft, gright, &result->rows.back());
            }
        }

#ifdef KEYPOINT
        for (auto const &row : result->rows){
            std::ostringstream _INFO;
            for (auto const &it : row) _INFO << it.type << " ";
            LOG(INFO) << _INFO.str();
        }
#endif

        bool flag = 0;
        for (auto const &row : result->rows)
            for (auto const &it : row) flag |= it.type == storage::UNDERPASS_PACKAGE || it.type == storage::PACKAGE;

        if (!flag) raiseError(MethodError, "empty result");
    }

    Basic::Basic (){
        parameters.registeR("penalty_for_liberal_storage", &_penalty_for_liberal_storage, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "非标准货架的权重，当方案出现过多非标准货架时，尝试增加该值；当期望方案添加一个短货架，或者拉长一个货架以充分利用空间时，尝试减少该值");
        parameters.registeR("penalty_for_liberal_endstorage", &_penalty_for_liberal_endstorage, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "不希望前后两端出现非标准货架时，尝试增加该值；允许前后两端出现非标准货架时，尝试减少该值");
        parameters.registeR("penalty_for_underpass_endstorage", &_penalty_for_underpass_endstorage, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "不希望前后两端出现underpass时，尝试增加该值；允许前后两端出现underpass时，尝试减少该值");
        parameters.registeR("penalty_for_collapsed_upright", &_penalty_for_collapsed_upright, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "不希望整体方案不对齐，且原因来自于某立柱被障碍物或者禁置区妨碍，尝试增加该值。希望通过放宽限制从而增加放置包裹数，则减少该值");
        parameters.registeR("penalty_for_occupied_beam", &_penalty_for_occupied_beam, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "不希望整体方案不对齐，且原因来自于某横梁被障碍物或者禁置区妨碍，尝试增加该值。希望通过放宽限制从而增加放置包裹数，则减少该值");
        parameters.registeR("penalty_for_large_storage_gap", &_penalty_for_large_storage_gap, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "不希望货架背靠背部分有过大缝隙，则增加该值。允许货架背靠背有缝隙（以放置柱子），则减少该值");
        parameters.registeR("penalty_for_irregular_upright", &_penalty_for_irregular_upright, make_pair(0, 6), {0, 1, 2, 3, 4, 5, 6}, "当允许货架列与列之间独立排布时，更倾向整体对齐则增加该值；更倾向尽可能放置包裹则减少该值；（这一权值最好不大于penalty_for_occupied_beam）");
        parameters.registeR("allow_individual_arrangement", &allow_individual_arrangement, make_pair(0, 1), {0, 1}, "是否允许货架列与列之间以一种相对独立的方案进行排布");
        parameters.registeR("quantity", &quantity, make_pair(0, 1000000), "");
    }

    void Basic::solve (Size_<2> const &_size, const StorageModel &_model, const ColisionSystem &_col, const ConnectionSystem &_con, py::dict info, LayoutResult* const _result){
        size = _size; model = &_model; col = &_col; con = &_con; result = _result;
        parameters.update(info);
#ifdef KEYPOINT
        LOG(INFO) << parameters.python();
#endif
        penalty_for_collapsed_upright = web_mapping[_penalty_for_collapsed_upright] * model->layers;
        penalty_for_occupied_beam = web_mapping[_penalty_for_occupied_beam] * model->layers;
        penalty_for_large_storage_gap = web_mapping[_penalty_for_large_storage_gap] * model->layers * 4;

        extra.reset(new ExtraConnectionSystem());

        //TODO converge_algorithm
        horizontalDP(vresult, &hresult);
        verticalDP(hresult, &vresult);
        horizontalDP(vresult, &hresult);
        verticalDP(hresult, &vresult);
        horizontalDP(vresult, &hresult);
        verticalDP(hresult, &vresult);
        horizontalDP(vresult, &hresult);

        //TODO decorate
        quantityMod();

        generateResult();
    }

    namespace{
        LayoutMethod::Registrar _1("basic", []{return new Basic();});
    }
}

