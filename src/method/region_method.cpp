#include "method.h"

namespace layout::method{
    class BasicRegionConnection : public RegionConnection{
        public :
            int moving_path_at_the_beginning = 0;
            int moving_path_in_the_end = 0;

            BasicRegionConnection (){
                parameters.registeR("moving_path_at_the_beginning", &moving_path_at_the_beginning, make_pair(0, 1), {0, 1}, "");
                parameters.registeR("moving_path_in_the_end", &moving_path_in_the_end, make_pair(0, 1), {0, 1}, "");
            }

            virtual py::dict solve (py::dict connection_item, const Local2D &region, const StorageModel &model, py::dict info){
                parameters.update(info);
#ifdef KEYPOINT
                LOG(INFO) << parameters.python();
                LOG(INFO) << connection_item;
#endif
                py::dict ret; Size_<2> size = region.fsize();
                if (connection_item.contains("soft_moving_paths")){
                    py::list item_list;
                    for (auto soft_moving_path : connection_item["soft_moving_paths"].cast<py::list>()){
                        Box box = soft_moving_path["box"].cast<pyBox>();
                        Point direction = soft_moving_path["direction"].cast<pyPoint>();
                        direction = region.fmap(Point(box.first + direction));
                        box = region.fmap(box);
                        int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                        if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                        //TODO moving_path structure update
                        if (box.first[0] != direction[0] && top - bottom > model.moving_path_width){
                            top -= model.moving_path_width; bottom += model.moving_path_width;
                            std::swap(top, bottom);
                            if (bottom >= top){
                                bottom = (bottom + top) / 2; top = bottom + 1;
                            }
                        }
                        if (box.first[1] != direction[1] && right - left > model.passage_width){
                            right -= model.passage_width; bottom += model.passage_width;
                            std::swap(left, right);
                            if (left >= right){
                                left = (left + right) / 2; right = left + 1;
                            }
                        }
                        py::dict moving_path;
                        moving_path["box"] = static_cast<pyBox>(region.unmap(Box{{left, bottom}, {right, top}}));
                        moving_path["direction"] = soft_moving_path["direction"];
                        item_list.append(moving_path);
                    }
                    ret["moving_paths"] = item_list;
                }
                else raiseError(InterfaceError, "soft_moving_paths");

                if (connection_item.contains("moving_paths")){
                    py::list item_list = ret["moving_paths"].cast<py::list>();
                    for (auto moving_path : connection_item["moving_paths"].cast<py::list>()){
                        Box box = moving_path["box"].cast<pyBox>();
                        Point direction = moving_path["direction"].cast<pyPoint>();
                        direction = region.fmap(Point(box.first + direction));
                        box = region.fmap(box);
                        int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                        if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                        item_list.append(moving_path);
                    }
                    ret["moving_paths"] = item_list;
                }
                else raiseError(InterfaceError, "moving_paths");

                if (connection_item.contains("blocks")){
                    py::list item_list;
                    for (auto block : connection_item["blocks"].cast<py::list>()){
                        Box box = block["box"].cast<pyBox>();
                        Point direction = block["direction"].cast<pyPoint>();
                        direction = region.fmap(Point(box.first + direction));
                        box = region.fmap(box);
                        int left = box.first[0], right = box.second[0], bottom = box.first[1], top = box.second[1];
                        if (left >= size[0] || right <= 0 || bottom >= size[1] || top <= 0) continue;
                        item_list.append(block);
                    }
                    ret["blocks"] = item_list;
                }
                else raiseError(InterfaceError, "blocks");

                //TODO shortcut
                ret["shortcut"] = py::list();

                return ret;
            }
    };

    namespace{
        RegionConnection::Registrar _1("basic", []{return new BasicRegionConnection();});
    }
}
