import layout
room = {
    "bbox": [[0, 0], [31100, 50000]],
    "obstacles" : [
       ([5400, 10000], [5700, 10000], [5700, 10300], [5400, 10300]),
       ([5400, 20000], [5700, 20000], [5700, 20300], [5400, 20300]),
       ([5400, 30000], [5700, 30000], [5700, 30300], [5400, 30300])
    ],
    "guards" : [
    ],
    "evitables" :[
    ]
}

region = ([0, 0], [31100, 50000])

connection_item = {
    "moving_paths" : [
    ],
    "blocks" : [
        {
            "box" : ([5400, 300], [5700, 10000]),
            "direction" : [0, 1]
        }
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
info['penalty_for_liberal_endstorage']['value'] = 10
info['penalty_for_underpass_endstorage']['value'] = 10
info['penalty_for_collapsed_upright']['value'] = 0
info['penalty_for_occupied_beam']['value'] = 10
info['penalty_for_large_storage_gap']['value'] = 10
info['penalty_for_irregular_upright']['value'] = 0
info['allow_individual_arrangement']['value'] = 0

params, param_list = layout.base_rack_setting(base_rack)