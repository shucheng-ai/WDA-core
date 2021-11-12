#pragma once
#include "common/geometry.hpp"
#include "storage/storage.h"
#include "utils/utils.hpp"

namespace layout::room{

    class ObstacleMap : public vector<Box>{

        public :

            bool testColision (const Box &b) const {
                bool flag = 0;
                for (auto const &b0 : *this)
                    flag |= b0.first[0] < b.second[0] && b.first[0] < b0.second[0] && b0.first[1] < b.second[1] && b.first[1] < b0.second[1];
                return flag;
            }

            int horiNext (const Box &b) const {
                int ret = numeric_limits<int>::max() >> 1;
                for (auto const &b0 : *this)
                    if (b0.first[1] < b.second[1] && b.first[1] < b0.second[1]){
                        if (b0.first[0] >= b.second[0]) ret = std::min(ret, b0.first[0] - b.second[0] + 1);
                        if (b.first[0] < b0.second[0]) ret = std::min(ret, b0.second[0] - b.first[0]);
                    }
                return ret;
            }

            int vertiNext (const Box &b) const {
                int ret = numeric_limits<int>::max() >> 1;
                for (auto const &b0 : *this)
                    if (b0.first[0] < b.second[0] && b.first[0] < b0.second[0]){
                        if (b0.first[1] >= b.second[1]) ret = std::min(ret, b0.first[1] - b.second[1] + 1);
                        if (b.first[1] < b0.second[1]) ret = std::min(ret, b0.second[1] - b.first[1]);
                    }
                return ret;
            }

            py::object python () const{
                py::list ret;
                for (auto &x : *this) ret.append(vector<pyPoint>({{x.first[0], x.first[1]}, {x.first[0], x.second[1]}, {x.second[0], x.second[1]}, {x.second[0], x.first[1]}}));
                return ret;
            }
    };

    class ColisionSystem{
        private :
            ObstacleMap obstacles, guards;
            ObstacleMap evitables;

            mutable int acc_flag, acc_next;

        public :

            static int eps;

            ColisionSystem (py::dict _room, py::dict history, const Local2D &region, py::dict info);

            void resetAcc (int flag) const {
                acc_flag = flag; acc_next = numeric_limits<int>::max() >> 1;
            }

            int accNext () const {return acc_next;}

            bool guardColision (int left, int right, int bottom, int top) const {
                Box b{{left, bottom}, {right, top}};
                acc_next = std::min(acc_next, acc_flag ? guards.vertiNext(b) : guards.horiNext(b));
                return guards.testColision(b);
            }

            bool obstacleColision (int left, int right, int bottom, int top) const {
                Box b{{left, bottom}, {right, top}};
                acc_next = std::min(acc_next, acc_flag ? obstacles.vertiNext(b) : obstacles.horiNext(b));
                return obstacles.testColision(b);
            }

            bool anyColision (int left, int right, int bottom, int top) const {
                return guardColision(left, right, bottom, top) || obstacleColision(left, right, bottom, top);
            }

            bool stabilityIgnore (int left, int right, int bottom, int top) const{
                Box b{{left, bottom}, {right, top}};
                acc_next = std::min(acc_next, acc_flag ? evitables.vertiNext(b) : evitables.horiNext(b));
                return evitables.testColision(b);
            }

            py::object python () const{
                py::dict ret;
                ret["obstacles"] = obstacles.python();
                ret["guards"] = guards.python();
                ret["evitables"] = evitables.python();
                return ret;
            }
    };

    using utils::RangeLowerBound;

    class ConnectionSystem{
        private :
            RangeLowerBound hori_block, hori_nec;
            RangeLowerBound verti_block, verti_moving_path, verti_shortcut;
        public :
            ConnectionSystem (py::dict _room, py::dict _moving_path, const Local2D &region, py::dict info);

            ConnectionSystem (){}

            void update (){
                hori_block.update(); hori_nec.update();
                verti_block.update(); verti_moving_path.update(); verti_shortcut.update();
            }

            bool horiSuf (int left, int right) const{
                return !hori_block.check(left, right);
            }

            bool horiNec (int left, int right) const{
                return hori_nec.check(left, right); 
            }

            bool vertiSuf (int left, int right) const{
                return !verti_block.check(left, right);
            }

            bool vertiNecForMovingPath (int left, int right) const{
                return verti_moving_path.check(left, right);
            }

            bool vertiNecForShortcut (int left, int right) const{
                return verti_shortcut.check(left, right);
            }

            bool vertiNec (int left, int right) const {
                return vertiNecForShortcut(left, right) || vertiNecForMovingPath(left, right);
            }

            py::object python () const{
                py::dict ret;
                ret["hori_block"] = static_cast<const vector<pi>&>(hori_block);
                ret["hori_nec"] = static_cast<const vector<pi>&>(hori_nec);
                ret["verti_block"] = static_cast<const vector<pi>&>(verti_block);
                ret["verti_moving_path"] = static_cast<const vector<pi>&>(verti_moving_path);
                ret["verti_shortcut"] = static_cast<const vector<pi>&>(verti_shortcut);
                return ret;
            }
    };

    using configurable::Configurable;

    class LayoutLocal : public Local2D{
        private :
            Configurable parameters;

        public :

            string direction = "vertical(rightwards)";

            LayoutLocal () {
                parameters.registeR("direction", &direction, {"vertical(leftwards)", "horizontal(upwards)", "vertical(rightwards)", "horizontal(downwards)"}, "");
            }

            py::object showParameters () {return parameters.python();}

            void make_region(Box _region, py::dict info);
    };

}

namespace layout{

    pyBox region_in_room(pyBox _region, py::dict room);
    py::object roomSetting(string const &name);
    py::object convert_wall(py::list wall);
    py::object connectionConvert(py::dict connection_item, py::dict params, py::dict info);
}
