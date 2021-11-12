#include "storage.h"

namespace layout::storage{

    py::object StorageModel::showDerived () const {
        py::dict ret;
        ret["info"] = info;
        ret["layers"] = layers;
        ret["allow_involvded_column"] = allow_involved_column;
        ret["storage_depth"] = storage_depth;
        ret["depth"] = depth;
        ret["passage_width"] = passage_width;
        ret["beam_depth"] = beam_depth;
        ret["storage_gap"] = storage_gap;

        ret["upright_width"] = upright_width;
        ret["package_width"] = package_width;

        ret["pallet_per_face"] = pallet_per_face;
        ret["underpass_pallet_per_face"] = underpass_pallet_per_face;

        ret["moving_path_width"] = moving_path_width;
        ret["shortcut_width"] = shortcut_width;
        ret["underpass_layers_for_moving_path"] = underpass_layers_for_moving_path;
        ret["underpass_layers_for_shortcut"] = underpass_layers_for_shortcut;
        return ret;
    }

    const int StorageModel::STORAGE_MAX = 100000;
}

namespace layout{

    using storage::StorageModel;

    py::object rackList (){
        py::list ret;
        for (auto &it : StorageModel::registry()) ret.append(it.first);
        return ret;
    }

    py::object rackSetting (string const &type){
        unique_ptr<StorageModel> model(StorageModel::create(type));
        model->updateParameters(py::dict());
        return model->showParameters();
    }
}
