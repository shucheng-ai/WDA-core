#include "method.h"

namespace layout{

    using storage::StorageModel;
    using room::ColisionSystem;
    using room::ConnectionSystem;
    using room::LayoutLocal;
    using method::LayoutMethod;
    using method::RegionConnection;
    using method::LayoutResult;
    using storage::PackageModel;
    using storage::SkeletonModel;

    py::object generate_plan_within_region (py::dict _room, pyBox _region, py::dict connection_item, py::dict history, string const &type, py::dict params, py::dict info){
        unique_ptr<StorageModel> model(StorageModel::create(type));
        model->updateParameters(params); model->extractParameters();

        _region = region_in_room(_region, _room);

        LayoutLocal region; region.make_region(_region, info);
        Size_<2> size = region.fsize();

        if (size[0] <= 0 || size[1] <= 0) raiseError(MethodError, "The region is invalid");

        ColisionSystem col(_room, history, region, info); 
        ConnectionSystem con(_room, connection_item, region, info);
#ifdef KEYPOINT
        LOG(INFO) << model->showParameters();
        LOG(INFO) << model->showDerived();
        LOG(INFO) << col.python();
        LOG(INFO) << con.python();
#endif
        unique_ptr<LayoutMethod> method(LayoutMethod::create("basic"));
        LayoutResult layout_result;
        method->solve(size, *model.get(), col, con, info, &layout_result);

        py::dict ret; ret["bbox"] = _region;

        pair<PackageModel, SkeletonModel> raw_result;
        model->generateResult(layout_result, &raw_result);
        raw_result.first.update();
        Box3D temp(raw_result.first);
        temp = region.unmap(temp);
        raw_result.first.unmap(region);
        raw_result.second.unmap(region);
        ret["base_rack"] = type;
        ret["packages"] = raw_result.first.python();
        ret["skeleton"] = raw_result.second.python();
        Box ob_bbox = raw_result.first.drop();
        ob_bbox.expand(raw_result.second.bbox());
        ret["obstacle"] = reinterpret_cast<pyBox&>(ob_bbox);

        py::list result_passage, result_moving_path;
        for (auto const &v : layout_result.vpoints){
            auto moving_path = region.unmap(Box{{0, v.first}, {size[0], v.second}});
            result_moving_path.append(reinterpret_cast<pyBox&>(moving_path));
        }
        ret["moving_paths"] = result_moving_path;
        Box guard;
        for (auto const &h : layout_result.hpoints){
            auto passage = region.unmap(Box{{h.first, 0}, {h.second, size[1]}});
            guard.expand(passage);
            result_passage.append(reinterpret_cast<pyBox&>(passage));
        }
        ret["passages"] = result_passage;
        ret["guard"] = reinterpret_cast<pyBox&>(guard);
        ret["shortcuts"] = py::list(); 
        return ret;
    }

    py::object algorithmSetting (string const &name){
        unique_ptr<LayoutMethod> solver(LayoutMethod::create("basic"));
        return solver->showParameters();
    }

    py::object region_connection (pyBox _region, py::dict connection_item, string const &type, py::dict params, py::dict info){
        unique_ptr<StorageModel> model(StorageModel::create(type));
        model->updateParameters(params); model->extractParameters();

        LayoutLocal region; region.make_region(_region, info);

        #ifdef KEYPOINT
            LOG(INFO) << region.showParameters();
        #endif

        unique_ptr<RegionConnection> method(RegionConnection::create("basic"));
        return method->solve(connection_item, region, *model.get(), info);
    }

    py::object regionConnectionSetting (string const &name){
        unique_ptr<RegionConnection> method(RegionConnection::create("basic"));
        return method->showParameters();
    }
}
