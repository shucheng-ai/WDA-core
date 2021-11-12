#include "method.h"
#include "utils/utils.hpp"

namespace layout::method{

    using storage::LEFTWARD;
    using storage::RIGHTWARD;
    using utils::RangeLowerBound;

    struct ExtraConnectionSystem{

        RangeLowerBound path, storage;

        void update (){
            path.update(); storage.update();
        }

        bool vertiPath (int left, int right) const{
            return path.check(left, right);
        }

        bool vertiStorage (int left, int right) const{
            return storage.check(left, right);
        }

        py::object python () const{
            py::dict ret;
            ret["storage"] = static_cast<const vector<pi>&>(storage);
            ret["path"] = static_cast<const vector<pi>&>(path);
            return ret;
        }

    };

    class Basic : public LayoutMethod{
        protected :

            void horizontalDP(const vector<VertiResult> &vresult, vector<HoriResult>* const hresult);
            int validUnit(int gleft, int gright, int cleft, int cright, const vector<VertiResult> &vresult, int pipe);
            vector<int> hori_acc, hori_last;

            void verticalDP(const vector<HoriResult> &hresult, vector<VertiResult>* const vresult);
            pi validUnit(int bottom, int top, const vector<HoriResult> &hresult);
            pi verti_last;
            int verti_acc;
            vector<pi> verti_offline;
            int countInvalidUpright(int bottom, int top, const vector<HoriResult> &hresult);
            int upright_acc, upright_last;

            void generateResult();
            void generateLocalResult(const vector<VertiResult> &vresult, int gleft, int gright, IndividualResult* const rows);

            unique_ptr<ExtraConnectionSystem> extra;

            vector<HoriResult> hresult;
            vector<VertiResult> vresult;

            int _penalty_for_liberal_storage = 3;
            int _penalty_for_liberal_endstorage = 2;
            int _penalty_for_underpass_endstorage = 2;
            int _penalty_for_collapsed_upright = 2;
            int _penalty_for_occupied_beam = 2;
            int _penalty_for_large_storage_gap = 2;
            int _penalty_for_irregular_upright = 0;
            int allow_individual_arrangement = 0;

            int ratio_for_bins = 10;
            int penalty_for_liberal_storage = 0;
            int penalty_for_liberal_endstorage = 0;
            int penalty_for_underpass_endstorage = 0;
            int penalty_for_collapsed_upright = 0;
            int penalty_for_occupied_beam = 0;
            int penalty_for_large_storage_gap = 0;
            int penalty_for_irregular_upright = 0; 

            int quantity = 0;

            const int web_mapping[7] = {0, 5, 10, 20, 40, 80, 160};

            bool validBeam (int cleft, int cright, int bottom, int top){
                return !model->allow_involved_column ? !col->anyColision(cleft, cright, bottom, top) :
                    !col->anyColision(cleft, cleft + model->beam_depth, bottom, top) && !col->anyColision(cright - model->beam_depth, cright, bottom, top);
            }

            void quantityMod();

        public :

            Basic();
            virtual void solve (Size_<2> const &_size, const StorageModel &_model, const ColisionSystem &_col, const ConnectionSystem &_con, py::dict info, LayoutResult* const _result) override;
    };

}
