#include "room.h"

namespace layout::room{

    namespace{

        void smoothPolygon (py::list points, vector<Point>* const result){
            //TODO robust 
            auto last = points[points.size() - 1].cast<pyPoint>();
            result->clear(); const int eps = ColisionSystem::eps;
            for (auto _p : points){
                auto p = _p.cast<pyPoint>();
                if (last[1] != p[1] && std::abs(last[0] - p[0]) >= eps){
                    int t = last[0] > p[0] ? -1 : 1;
                    for (int i = last[0]; t * (p[0] - i) >= eps; i += t * eps)
                        result->push_back(Point{i, (int)yFromLineX(last, p, i)});
                }
                else result->emplace_back(last);
                last = p;
            }
        }

        void patternRegularize (vector<Point> const &points, ObstacleMap* const plist){
            vector<int> s;
            for (auto &p : points) s.push_back(p[0]);
            sort(s.begin(), s.end());
            s.resize(unique(s.begin(), s.end()) - s.begin());
            for (int i = 1; i < s.size(); ++i){
                int left = s[i - 1], right = s[i];
                int top = numeric_limits<int>::min(), bottom = numeric_limits<int>::max();
                auto last = points.back();
                for (auto &p: points){
                    if (p[0] != last[0]){
                        if (std::min(p[0], last[0]) <= left && left < std::max(p[0], last[0])){
                            top = std::max(top, int(yFromLineX(p, last, left) + 0.5));
                            bottom = std::min(bottom, int(yFromLineX(p, last, left)));
                        }
                        if (std::min(p[0], last[0]) < right && right <= std::max(p[0], last[0])){
                            top = std::max(top, int(yFromLineX(p, last, right) + 0.5));
                            bottom = std::min(bottom, int(yFromLineX(p, last, right)));
                        }
                    }
                    last = p;
                }
                Box b = Box{{left, bottom}, {right, top}};
                if (!b.empty()) plist->push_back(b);
            }
        }

        void fmap (vector<Box> &blist, const Local2D &region){
            auto last = blist.begin(); Size_<2> size = region.fsize();
            for (auto &b : blist){
                *last = region.fmap(b);
                if (last->first[0] < size[0] && last->first[1] < size[1] && last->second[0] > 0 && last->second[1] > 0) ++last;
            }
            blist.resize(last - blist.begin());
        }
    }

    ColisionSystem::ColisionSystem (py::dict _room, py::dict history, const Local2D &region, py::dict info){
        if (_room.contains("guards"))
            for (auto fixture : _room["guards"].cast<py::list>()){
                vector<Point> points; smoothPolygon(fixture.cast<py::list>(), &points);
                patternRegularize(points, &guards);
            }
        else raiseError(InterfaceError, "guards");

        if (_room.contains("obstacles"))
            for (auto fixture : _room["obstacles"].cast<py::list>()){
                vector<Point> points; smoothPolygon(fixture.cast<py::list>(), &points);
                patternRegularize(points, &obstacles);
            }
        else raiseError(InterfaceError, "obstacles");

        if (_room.contains("evitables"))
            for (auto fixture : _room["evitables"].cast<py::list>()){
                vector<Point> points; smoothPolygon(fixture.cast<py::list>(), &points);
                patternRegularize(points, &evitables);
            }
        else raiseError(InterfaceError, "evitables");

        try{
            for (auto _plan : history){
                auto plan = _plan.second.cast<py::dict>();
                guards.emplace_back(plan["guard"].cast<pyBox>());
                obstacles.emplace_back(plan["obstacle"].cast<pyBox>());
            }
        }
        catch (...){
            raiseError(InterfaceError, "history");
        }

        fmap(guards, region);
        fmap(obstacles, region);
        fmap(evitables, region);
    }

    ConnectionSystem::ConnectionSystem (py::dict _room, py::dict connection_item, const Local2D &region, py::dict info){
        Size_<2> size = region.fsize();
        if (connection_item.contains("moving_paths")){
            for (auto moving_path : connection_item["moving_paths"].cast<py::list>()){
                Box box = moving_path["box"].cast<pyBox>();
                Point direction = moving_path["direction"].cast<pyPoint>();
                direction = region.a * direction;
                box = region.fmap(box);
                int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                if (direction[0])
                    verti_moving_path.emplace_back(bottom, top);
                else
                    hori_nec.emplace_back(left, right);
            }
        }
        else raiseError(InterfaceError, "moving_paths");

        if (connection_item.contains("blocks")){
            for (auto block : connection_item["blocks"].cast<py::list>()){
                Box box = block["box"].cast<pyBox>();
                Point direction = block["direction"].cast<pyPoint>();
                direction = region.a * direction;
                box = region.fmap(box);
                int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                if (direction[0])
                    verti_block.emplace_back(bottom, top);
                else
                    hori_block.emplace_back(left, right);
            }
        }
        else raiseError(InterfaceError, "blocks");

        if (connection_item.contains("shortcuts")){
            for (auto shortcut : connection_item["shortcuts"].cast<py::list>()){
                Box box = shortcut["box"].cast<pyBox>();
                Point direction = shortcut["direction"].cast<pyPoint>();
                direction = region.a * direction;
                box = region.fmap(box);
                int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                if (direction[0])
                    verti_shortcut.emplace_back(bottom, top);
            }
        }
        //else raiseError(InterfaceError, "shortcuts");

        update();
    }

    namespace{
        Matrix left_rotation{0, 1, -1, 0};
        Matrix right_rotation{0, -1, 1, 0};
        Matrix mirror{-1, 0, 0, 1};
    }

    void LayoutLocal::make_region (Box _region, py::dict info){

        box = _region; parameters.update(info);

        switch (hash_compile_time(direction.c_str())){
            case "vertical(leftwards)"_hash :
                a = a_ = mirror;
                break;
            case "horizontal(upwards)"_hash :
                a = left_rotation;
                a_ = right_rotation;
                break;
            case "horizontal(downwards)"_hash :
                a = right_rotation;
                a_ = left_rotation;
                break;
            case "vertical(rightwards)"_hash:
                break;
            default :
                break;
        }
    }

    int ColisionSystem::eps = 500;
}

namespace layout{

    using room::LayoutLocal;

    pyBox region_in_room (pyBox _region, py::dict room){
        Box region_box = _region;
        try{
            region_box.shrink(room["bbox"].cast<pyBox>());
        }
        catch (...){
            raiseError(InterfaceError, "room[\"bbox\"] is invalid");
        }
        if (region_box.empty()) raiseError(MethodError, "the region is invalid");
        return region_box;
    }

    py::object roomSetting (string const &name){
        LayoutLocal region;
        return region.showParameters();
    }

    using room::smoothPolygon;
    using room::patternRegularize;
    using room::ObstacleMap; 

    py::object convert_wall (py::list wall){
        Box bbox;
        try {
            for (auto p : wall) bbox.expand(p.cast<pyPoint>());
            if (bbox.empty()) throw std::exception();
        }
        catch(...){
            raiseError(InterfaceError, "invalid wall");
        }
        vector<Point> points; smoothPolygon(wall, &points);
        //convert into clockwise
        long long sum = cross2D(points.back(), points.front());
        for (auto it = points.begin(); it < points.end() - 1; it++) sum += cross2D(*it, *(it + 1));
        if (sum < 0) reverse(points.begin(), points.end());
        //convert wall into squares
        auto next = [&points](int x) {return x == points.size() - 1 ? 0 : x + 1;};
        auto isOntheEdge = [&bbox](Point const &p) {
            if (p[1] == bbox.second[1]) return 4;
            if (p[0] == bbox.second[0]) return 3;
            if (p[1] == bbox.first[1]) return 2;
            if (p[0] == bbox.first[0]) return 1;
            return 0;
        };
        int start = -1;
        for (int i = 0; i < points.size(); i++)
            if (isOntheEdge(points[i])){
                start = i; break;
            }
        if (start == -1) raiseError(UnknownError, "Do not stand at my grave and cry. I am not there, I did not die.");
        int i = start, color = isOntheEdge(points[i]); ObstacleMap ret;
        auto edgePoint = [&bbox, &color](Point const &p){
            auto ret = p;
            switch (color){
                case 4 :
                    ret[1] = bbox.second[1]; break;
                case 3 :
                    ret[0] = bbox.second[0]; break;
                case 2 :
                    ret[1] = bbox.first[1]; break;
                case 1 :
                    ret[0] = bbox.first[0]; break;
            }
            return ret;
        };
        do
        {
            patternRegularize({points[i], points[next(i)], edgePoint(points[next(i)]), edgePoint(points[i])}, &ret);
            i = next(i);
            if (int temp = isOntheEdge(points[i])) color = temp;
        }while (i != start);
        return ret.python();
    }

    py::object connectionConvert(py::dict connection_item, py::dict params, py::dict info){
        py::dict ret = connection_item;
        LOG(INFO) << ret;
        return ret;
    }
}
