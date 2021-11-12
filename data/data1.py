import layout
room = {
    "bbox": [[0, 0], [30000, 50000]],
    "obstacles" : [
    ],
    "guards" : [
    ],
    "evitables" :[
    ]
}

region = ([0, 0], [30000, 50000])

connection_item = {
    "moving_paths" : [
        {
            "box" : ([0, 300], [3000, 10000]),
            "direction" : [0, 1]
        },
        {
            "box" : ([0, 4000], [70000, 6000]),
            "direction" : [1, 0]
        },
        {
            "box" : ([0, 14000], [70000, 16000]),
            "direction" : [1, 0]
        }
    ],
    "blocks" : [
    ],
    "shortcuts" : [
    ]
}

history = {}

base_rack = "apr"

info = {
    **layout.base_algorithm_setting("")[0],
    **layout.base_region_setting("")[0],
    **layout.base_region_connection_setting("")[0]
}

info['ratio_for_bins']['value'] = 10
info['penalty_for_liberal_storage']['value'] = 10
info['penalty_for_liberal_endstorage']['value'] = 0
info['penalty_for_underpass_endstorage']['value'] = 0
info['penalty_for_collapsed_upright']['value'] = 10
info['penalty_for_occupied_beam']['value'] = 10
info['penalty_for_large_storage_gap']['value'] = 10
info['penalty_for_irregular_upright']['value'] = 0
info['allow_individual_arrangement']['value'] = 0

params, param_list = layout.base_rack_setting(base_rack)