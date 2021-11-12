#include "basic.h"

namespace layout::method{
    void Basic::quantityMod (){
        int sum = 0;
        if (!quantity) return;
        for (int num = 0; num < hresult.size(); ++num){
            const auto &h = hresult[num];
            for (int i = h.left; i + model->storage_depth <= h.right; i += model->storage_depth + model->storage_gap){
                int cleft = i, cright = i + model->storage_depth;
                int gleft = h.direction == RIGHTWARD ? cleft : h.left - model->passage_width;
                int gright = h.direction == RIGHTWARD ? h.right + model->passage_width : cright;
                
                for (const auto &v : vresult){
                    int bottom = v.bottom, top = v.top;
                    if (v.type == storage::PACKAGE && !col->obstacleColision(gleft, gright, bottom, top) && !col->guardColision(cleft, cright, bottom, top))
                        sum += model->layers;
                    if (v.type == storage::UNDERPASS_PACKAGE && !col->obstacleColision(gleft, gright, bottom, top) && !col->guardColision(cleft, cright, bottom, top))
                        sum += model->underpass_layers_for_shortcut | model->underpass_layers_for_moving_path;
                }
            }
            if (sum > quantity){
                hresult.resize(num + 1); break;
            }
        }
    }
}
