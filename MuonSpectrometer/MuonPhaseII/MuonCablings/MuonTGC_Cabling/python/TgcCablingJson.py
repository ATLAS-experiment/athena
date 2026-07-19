# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

###     Simple script to generate a TGC cabling map


import json

cabling_data = {
    "Detector": "TGC",
    "RunPeriod": 4,
    "SchemaVersion": "1.0",
    "CablingData": []
}


def L(value):
    if isinstance(value, list):
        return value

    if isinstance(value, int):
        return [value]

    return [
        int(x)
        for x in str(value).replace(" ", "").replace(".", "").split(",")
        if x != ""
    ]


FPGA_LINK_NUMBERING = {
    (11, 1): 20,
    (4, 1): 6,
    (10, 2): 19,
    (2, 1): 2,
    (3, 2): 5,
    (2, 2): 3,
    (1, 1): 0,
    (3, 1): 4,
    (1, 2): 1,
    (13, 2): 25,
    (15, 1): 28,
    (13, 1): 24,
    (15, 2): 29,
    (14, 1): 26,
    (14, 2): 27,
    (16, 2): 31,
    (16, 1): 30,
    (17, 1): 32,
    (17, 2): 33,
    (18, 1): 34,
    (18, 2): 35,
    (12, 1): 22,
    (20, 2): 39,
    (12, 2): 23,
    (10, 1): 18,
    (9, 2): 17,
    (9, 1): 16,
    (8, 1): 14,
    (8, 2): 15,
    (7, 1): 12,
    (7, 2): 13,
    (6, 1): 10,
    (6, 2): 11,
    (20, 1): 38,
    (21, 1): 40,
    (21, 2): 41,
    (22, 2): 43,
    (22, 1): 42,
    (23, 2): 45,
    (23, 1): 44,
    (24, 2): 47,
    (24, 1): 46,
    (25, 1): 48,
    (25, 2): 49,
    (26, 1): 50,
    (26, 2): 51,
    (27, 1): 52,
    (27, 2): 53,
    (5, 1): 8,
    (4, 2): 7,
    (11, 2): 21,
    (5, 2): 9,
    (19, 2): 37,
    (19, 1): 36,
    (29, 1): 56,
    (28, 1): 54,
    (28, 2): 55,
    (29, 2): 57,
    ("EI-1*", 1): 58,
    ("EI-1*", 2): 59,
    ("EI-2*", 1): 60,
    ("EI-2*", 2): 61,
}


T1E_FPGA_MAP = {
    "EW": {
        0: {1: L("6,1"), 2: L("6,1"), 3: L("6,1")},
        1: {1: L("6,1"), 2: L("6,1"), 3: L("6,1")},
        2: {1: L("6,1"), 2: L("6,1"), 3: L("6,1")},
        3: {1: L("6,1"), 2: L("6,1"), 3: L("6,1")},
        4: {1: L("6,1"), 2: L("6,1"), 3: L("7,2")},
        5: {1: L("7,2"), 2: L("7,2"), 3: L("7,2")},
        6: {1: L("7,2"), 2: L("7,2"), 3: L("7,2")},
        7: {1: L("7,2"), 2: L("7,2"), 3: L("7,2")},
        8: {1: L("8,3"), 2: L("8,3"), 3: L("8,3")},
        9: {1: L("8,3"), 2: L("8,3"), 3: L("8,3")},
        10: {1: L("8,3"), 2: L("8,3"), 3: L("8,3")},
        11: {1: L("8,3"), 2: L("8,3"), 3: L("8,3")},
        12: {1: L("9,4"), 2: L("9,4"), 3: L("9,4")},
        13: {1: L("9,4"), 2: L("9,4"), 3: L("9,4")},
    },
    "ES": {
        0: {1: L("9,10"), 3: L("9,10")},
        1: {1: L("9,10"), 3: L("9,10")},
        2: {1: L("9,10"), 3: L("9,10")},
        3: {1: L("9,10"), 3: L("9,10")},
        4: {1: L("10,11"), 3: L("10,11")},
        5: {1: L("10,11"), 3: L("10,11")},
        6: {1: L("10,11"), 3: L("10,11")},
        7: {1: L("10,11"), 3: L("10,11")},
    }
}


T1E_PPASIC_MAP = {
    "EW": {
        0: {1: L(1), 2: L(0), 3: L(2)},
        1: {1: L(5), 2: L(4), 3: L(3)},
        2: {1: L(7), 2: L(6), 3: L(8)},
        3: {1: L(9), 2: L(10), 3: L(11)},
        4: {1: L(15), 2: L(14), 3: L(0)},
        5: {1: L(4), 2: L(5), 3: L(1)},
        6: {1: L(7), 2: L(6), 3: L(8)},
        7: {1: L(13), 2: L(12), 3: L(9)},
        8: {1: L(5), 2: L(4), 3: L(0)},
        9: {1: L(7), 2: L(6), 3: L(1)},
        10: {1: L(13), 2: L(12), 3: L(10)},
        11: {1: L(15), 2: L(14), 3: L(11)},
        12: {1: L(5), 2: L(4), 3: L(0)},
        13: {1: L(7), 2: L(6), 3: L(1)},
    },
    "ES": {
        0: {1: L("15,15"), 3: L("14,14")},
        1: {1: L("13,13"), 3: L("12,12")},
        2: {1: L("9,9"), 3: L("8,8")},
        3: {1: L("11,11"), 3: L("10,10")},
        4: {1: L("7,13"), 3: L("6,12")},
        5: {1: L("5,5"), 3: L("4,4")},
        6: {1: L("3,9"), 3: L("2,8")},
        7: {1: L("1,1"), 3: L("0,0")},
    }
}


T1F_FPGA_MAP = {
    "FW": {
        0: {1: L(4), 2: L(4), 3: L(4)},
        1: {1: L(4), 2: L(4), 3: L(4)},
        2: {1: L(5), 2: L(5), 3: L(5)},
        3: {1: L(5), 2: L(5), 3: L(5)},
        4: {1: L(5), 2: L(5), 3: L(5)},
        5: {1: L(5), 2: L(5), 3: L(5)},
        6: {1: L(11), 2: L(11), 3: L(5)},
    },
    "FS": {
        0: {1: L(11), 3: L(11)},
        1: {1: L(11), 3: L(11)},
    }
}


T1F_PPASIC_MAP = {
    "FW": {
        0: {1: L(13), 2: L(12), 3: L(10)},
        1: {1: L(15), 2: L(14), 3: L(11)},
        2: {1: L(5), 2: L(4), 3: L(0)},
        3: {1: L(7), 2: L(6), 3: L(1)},
        4: {1: L(13), 2: L(12), 3: L(8)},
        5: {1: L(15), 2: L(14), 3: L(9)},
        6: {1: L(3), 2: L(2), 3: L(11)},
    },
    "FS": {
        0: {1: L(15), 3: L(14)},
        1: {1: L(7), 3: L(6)},
    }
}


T2T3E_FPGA_MAP = {
    "EW": {
        0: {4: L(20), 5: L(20), 6: L(20), 7: L(20)},
        1: {4: L(20), 5: L(20), 6: L(20), 7: L(20)},
        2: {4: L("21,12"), 5: L("21,12"), 6: L("21,12"), 7: L("21,12")},
        3: {4: L("21,12"), 5: L("21,12"), 6: L("21,12"), 7: L("21,12")},
        4: {4: L("21,12"), 5: L("21,12"), 6: L("21,12"), 7: L("21,12")},
        5: {4: L("21,12"), 5: L("21,12"), 6: L("22,13"), 7: L("22,13")},
        6: {4: L("22,13"), 5: L("22,13"), 6: L("22,13"), 7: L("22,13")},
        7: {4: L("22,13"), 5: L("22,13"), 6: L("22,13"), 7: L("22,13")},
        8: {4: L("22,13"), 5: L("22,13"), 6: L("22,13"), 7: L("22,13")},
        9: {4: L("22,13"), 5: L("22,13"), 6: L("23,14"), 7: L("23,14")},
        10: {4: L("23,14"), 5: L("23,14"), 6: L("23,14"), 7: L("23,14")},
        11: {4: L("23,14"), 5: L("23,14"), 6: L("23,14"), 7: L("23,14")},
        12: {4: L("23,14"), 5: L("23,14"), 6: L("23,14"), 7: L("23,14")},
        13: {4: L("23,14"), 5: L("23,14"), 6: L("24,15"), 7: L("24,15")},
        14: {4: L("24,15"), 5: L("24,15"), 6: L("24,15"), 7: L("24,15")},
        15: {4: L("24,15"), 5: L("24,15"), 6: L("24,15"), 7: L("24,15")},
        16: {4: L("24,15"), 5: L("24,15"), 6: L("24,15"), 7: L("24,15")},
        17: {4: L("24,15"), 5: L("24,15"), 6: L("25,16"), 7: L("25,16")},
        18: {4: L("25,16"), 5: L("25,16"), 6: L("25,16"), 7: L("25,16")},
        19: {4: L("25,16"), 5: L("25,16")},
    },
    "ES": {
        0: {4: L("25,16"), 5: L("25,16"), 6: L("25,16"), 7: L("25,16")},
        1: {4: L("25,16"), 5: L("25,16"), 6: L("25,16"), 7: L("25,16")},
        2: {4: L("26,17"), 5: L("26,17"), 6: L("26,17"), 7: L("26,17")},
        3: {4: L("26,17"), 5: L("26,17"), 6: L("26,17"), 7: L("26,17")},
        4: {4: L("26,17"), 5: L("26,17"), 6: L("26,17"), 7: L("26,17")},
        5: {4: L("26,17"), 5: L("26,17"), 6: L("26,17"), 7: L("26,17")},
        6: {4: L("27,18"), 5: L("27,18"), 6: L("27,18"), 7: L("27,18")},
        7: {4: L("27,18"), 5: L("27,18"), 6: L("27,18"), 7: L("27,18")},
        8: {4: L("27,18"), 5: L("27,18"), 6: L("27,18"), 7: L("27,18")},
        9: {4: L("27,18"), 5: L("27,18"), 6: L("27,18"), 7: L("27,18")},
    }
}


T2T3E_PPASIC_MAP = {
    "EW": {
        0: {4: L("3,7"), 5: L("2,6"), 6: L("1,5"), 7: L("0,4")},
        1: {4: L("11,15"), 5: L("10,14"), 6: L("9,13"), 7: L("8,12")},
        2: {4: L(1), 5: L(0), 6: L(5), 7: L(4)},
        3: {4: L(3), 5: L(2), 6: L(7), 7: L(6)},
        4: {4: L(9), 5: L(8), 6: L(13), 7: L(12)},
        5: {4: L(11), 5: L(10), 6: L(1), 7: L(0)},
        6: {4: L(5), 5: L(4), 6: L(3), 7: L(2)},
        7: {4: L(7), 5: L(6), 6: L(9), 7: L(8)},
        8: {4: L(13), 5: L(12), 6: L(15), 7: L(14)},
        9: {4: L(11), 5: L(10), 6: L(1), 7: L(0)},
        10: {4: L(5), 5: L(4), 6: L(3), 7: L(2)},
        11: {4: L(7), 5: L(6), 6: L(9), 7: L(8)},
        12: {4: L(13), 5: L(12), 6: L(15), 7: L(14)},
        13: {4: L(11), 5: L(10), 6: L(1), 7: L(0)},
        14: {4: L(5), 5: L(4), 6: L(3), 7: L(2)},
        15: {4: L(7), 5: L(6), 6: L(9), 7: L(8)},
        16: {4: L(13), 5: L(12), 6: L(15), 7: L(14)},
        17: {4: L(11), 5: L(10), 6: L(1), 7: L(0)},
        18: {4: L(5), 5: L(4), 6: L(7), 7: L(6)},
        19: {4: L(2), 5: L(3)},
    },
    "ES": {
        0: {4: L("15,15"), 5: L("14,14"), 6: L("11,11"), 7: L("10,10")},
        1: {4: L("9,9"), 5: L("8,8"), 6: L("13,13"), 7: L("12,12")},
        2: {4: L("3,3"), 5: L("2,2"), 6: L("7,7"), 7: L("6,6")},
        3: {4: L("1,1"), 5: L("0,0"), 6: L("5,5"), 7: L("4,4")},
        4: {4: L("15,15"), 5: L("14,14"), 6: L("11,11"), 7: L("10,10")},
        5: {4: L("9,9"), 5: L("8,8"), 6: L("13,13"), 7: L("12,12")},
        6: {4: L("3,3"), 5: L("2,2"), 6: L("7,7"), 7: L("6,6")},
        7: {4: L("1,1"), 5: L("0,0"), 6: L("5,5"), 7: L("4,4")},
        8: {4: L("15,15"), 5: L("14,14"), 6: L("11,11"), 7: L("10,10")},
        9: {4: L("9,9"), 5: L("8,8"), 6: L("13,13"), 7: L("12,12")},
    }
}


T2T3F_FPGA_MAP = {
    "FW": {
        0: {4: L(28), 5: L(28), 6: L(28), 7: L(28)},
        1: {4: L(28), 5: L(28), 6: L(28), 7: L(28)},
        2: {4: L(28), 5: L(28), 6: L(28), 7: L(28)},
        3: {4: L(29), 5: L(29), 6: L(29), 7: L(29)},
        4: {4: L(29), 5: L(29), 6: L(29), 7: L(29)},
        5: {4: L(19), 5: L(19), 6: L(19), 7: L(19)},
        6: {4: L(19), 5: L(19), 6: L(19), 7: L(19)},
        7: {4: L(19), 5: L(19), 6: L(19), 7: L(19)},
    },
    "FS": {
        0: {4: L(29), 5: L(29), 6: L(29), 7: L(29)},
        1: {4: L(29), 5: L(29), 6: L(29), 7: L(29)},
    }
}


T2T3F_PPASIC_MAP = {
    "FW": {
        0: {4: L(1), 5: L(0), 6: L(5), 7: L(4)},
        1: {4: L(3), 5: L(2), 6: L(7), 7: L(6)},
        2: {4: L(15), 5: L(14), 6: L(11), 7: L(10)},
        3: {4: L(1), 5: L(0), 6: L(5), 7: L(4)},
        4: {4: L(3), 5: L(2), 6: L(7), 7: L(6)},
        5: {4: L(15), 5: L(14), 6: L(11), 7: L(10)},
        6: {4: L(3), 5: L(2), 6: L(7), 7: L(6)},
        7: {4: L(1), 5: L(0), 6: L(5), 7: L(4)},
    },
    "FS": {
        0: {4: L(15), 5: L(14), 6: L(11), 7: L(10)},
        1: {4: L(9), 5: L(8), 6: L(13), 7: L(12)},
    }
}


station_configs = {
    "T1F": {
        "eta_range": range(1),                 # 1 only
        "phi_range": range(1, 25),             # 1...24
        "gap_range": [[1, 2, 3], [1, 3]],      # [wire][strip]
        "wasd_range": [[6, 5, 4, 3, 2, 1, 0]], # T1 chamber has 7 Wire ASDs
        "fpga_map": T1F_FPGA_MAP,
        "ppasic_map": T1F_PPASIC_MAP
    },
    "T1E": {
        "eta_range": range(4),                 # 1...4
        "phi_range": range(1, 49),             # 1...48
        "gap_range": [[1, 2, 3], [1, 3]],      # [wire][strip]
        "wasd_range": [[13, 12, 11, 10, 9, 8],
                       [7, 6, 5, 4],
                       [3, 2],
                       [1, 0]],
        "fpga_map": T1E_FPGA_MAP,
        "ppasic_map": T1E_PPASIC_MAP
    },
    "T2F": {
        "eta_range": range(1),                    # 1 only
        "phi_range": range(1, 25),                # 1...24
        "gap_range": [[1, 2], [1, 2]],            # [wire][strip]
        "wasd_range": [[7, 6, 5, 4, 3, 2, 1, 0]],
        "fpga_map": T2T3F_FPGA_MAP,
        "ppasic_map": T2T3F_PPASIC_MAP
    },
    "T2E": {
        "eta_range": range(5),                       # 1...5
        "phi_range": range(1, 49),                   # 1...48
        "gap_range": [[1, 2], [1, 2]],               # [wire][strip]
        "wasd_range": [[19, 18, 17, 16, 15, 14, 13],
                       [12, 11, 10, 9, 8, 7, 6],
                       [5, 4],
                       [3, 2],
                       [1, 0]],
        "fpga_map": T2T3E_FPGA_MAP,
        "ppasic_map": T2T3E_PPASIC_MAP
    },
    "T3F": {
        "eta_range": range(1),                    # 1 only
        "phi_range": range(1, 25),                # 1...24
        "gap_range": [[1, 2], [1, 2]],            # [wire][strip]
        "wasd_range": [[7, 6, 5, 4, 3, 2, 1, 0]],
        "fpga_map": T2T3F_FPGA_MAP,
        "ppasic_map": T2T3F_PPASIC_MAP
    },
    "T3E": {
        "eta_range": range(5),                     # 1...5
        "phi_range": range(1, 49),                 # 1...48
        "gap_range": [[1, 2], [1, 2]],             # [wire][strip]
        "wasd_range": [[18, 17, 16, 15, 14, 13],
                       [12, 11, 10, 9, 8, 7, 6],
                       [5, 4],
                       [3, 2],
                       [1, 0]],
        "fpga_map": T2T3E_FPGA_MAP,
        "ppasic_map": T2T3E_PPASIC_MAP
    },
    "T4E": {
        "eta_range": range(1),                 # 1 only
        "phi_range": range(1, 22),             # 1...21
        "gap_range": [[1, 2, 3], [1, 2, 3]],   # [wire][strip]
        "wasd_range": [[1, 0]]                 # T12 chamber has 2 Wire ASDs
    }
}


# connected channel start and end number (0...15) for each ASD
endcap_wire_channel_in_asd = {
    "T1F": {
        f"FW{station_configs['T1F']['wasd_range'][0][0]}": [[7, 15], [8, 15], [7, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][1]}": [[0, 15], [0, 15], [0, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][2]}": [[0, 15], [0, 15], [0, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][3]}": [[0, 15], [0, 15], [0, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][4]}": [[0, 15], [0, 15], [0, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][5]}": [[0, 15], [0, 15], [0, 15]],
        f"FW{station_configs['T1F']['wasd_range'][0][6]}": [[0, 15], [0, 15], [0, 15]]
    },
    "T1E": {
        f"EW{station_configs['T1E']['wasd_range'][0][0]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][0][1]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][0][2]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][0][3]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][0][4]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][0][5]}": [[0, 11], [0, 10], [0, 10]],
        f"EW{station_configs['T1E']['wasd_range'][1][0]}": [[1, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][1][1]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][1][2]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][1][3]}": [[0, 13], [0, 13], [0, 13]],
        f"EW{station_configs['T1E']['wasd_range'][2][0]}": [[5, 15], [5, 15], [5, 15]],
        f"EW{station_configs['T1E']['wasd_range'][2][1]}": [[4, 15], [4, 15], [4, 15]],
        f"EW{station_configs['T1E']['wasd_range'][3][0]}": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T1E']['wasd_range'][3][1]}": [[0, 7],  [0, 7],  [0, 7]]
    },
    "T2F": {
        f"FW{station_configs['T2F']['wasd_range'][0][0]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][1]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][2]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][3]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][4]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][5]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][6]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T2F']['wasd_range'][0][7]}": [[0, 12], [0, 12]]
    },
    "T2E": {
        f"EW{station_configs['T2E']['wasd_range'][0][0]}": [[1, 15], [1, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][2]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][3]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][4]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][5]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][0][6]}": [[0, 14], [0, 14]],
        f"EW{station_configs['T2E']['wasd_range'][1][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][2]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][3]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][4]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][5]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][1][6]}": [[0, 6],  [0, 6]],
        f"EW{station_configs['T2E']['wasd_range'][2][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][2][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][3][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][3][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][4][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T2E']['wasd_range'][4][1]}": [[0, 15], [0, 15]]
    },
    "T3F": {
        f"FW{station_configs['T3F']['wasd_range'][0][0]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][1]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][2]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][3]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][4]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][5]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][6]}": [[0, 15], [0, 15]],
        f"FW{station_configs['T3F']['wasd_range'][0][7]}": [[0, 9],  [0, 9]]
    },
    "T3E": {
        f"EW{station_configs['T3E']['wasd_range'][0][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][0][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][0][2]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][0][3]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][0][4]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][0][5]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][2]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][3]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][4]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][5]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][1][6]}": [[0, 9],  [0, 9]],
        f"EW{station_configs['T3E']['wasd_range'][2][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][2][1]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][3][0]}": [[0, 15], [0, 15]],
        f"EW{station_configs['T3E']['wasd_range'][3][1]}": [[0, 13], [0, 13]],
        f"EW{station_configs['T3E']['wasd_range'][4][0]}": [[1, 15], [1, 15]],
        f"EW{station_configs['T3E']['wasd_range'][4][1]}": [[0, 15], [0, 15]]
    },
    "T4E": {
        f"EW{station_configs['T4E']['wasd_range'][0][0]}N": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T4E']['wasd_range'][0][1]}N": [[0, 15], [0, 15], [0, 15]],
        f"EW{station_configs['T4E']['wasd_range'][0][0]}S": [[11, 15], [11, 15], [11, 15]],
        f"EW{station_configs['T4E']['wasd_range'][0][1]}S": [[0, 15],  [0, 15],  [0, 15]]
    }
}


EIL4_ASIDE_CHAMBERS = [
    {"type": "Normal",  "direction": "forward"},
    {"type": "Special", "direction": "backward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "backward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "backward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "backward"},
    {"type": "Special", "direction": "forward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Special", "direction": "backward"},
    {"type": "Special", "direction": "forward"},
    {"type": "Special", "direction": "backward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "forward"},
    {"type": "Normal",  "direction": "backward"},
    {"type": "Special", "direction": "forward"},
    {"type": "Special", "direction": "backward"},
    {"type": "Special", "direction": "forward"}
]


def calculate_slid(station_name, side, phi):
    """
    return SLID from stationName, side, phi
    """
    slid = (0b1 << 5) if side > 0 else 0b0

    if station_name == "T1F" or station_name == "T2F" or station_name == "T3F":
        slid += (phi % 24)
        return slid

    elif station_name == "T1E" or station_name == "T2E" or station_name == "T3E":
        slid += ((phi + 1) // 2) % 24
        return slid

    elif station_name == "T4E":
        conn = [[0, 1], [1, 2], [2, 3], [3, 4],
                [5, 6], [6, 7], [7, 8], [8, 9],
                [9, 10], [10, 11], [11, 12], [12, 13],
                [13, 14], [14, 15], [15, 16], [17, 18],
                [18, 19], [19, 20], [21, 22], [22, 23],
                [23, 0]]
        eislid = [conn[phi - 1][0] + slid, conn[phi - 1][1] + slid]
        return eislid

    return slid


def BW_global_layer(station_name, gap):
    if station_name.startswith("T1"):
        return gap

    if station_name.startswith("T2"):
        return 3 + gap

    if station_name.startswith("T3"):
        return 5 + gap

    raise ValueError(f"Unsupported BW station: {station_name}")


def select_by_local_phi(values, local_phi):
    if len(values) == 1:
        return values[0]

    if local_phi >= len(values):
        raise ValueError(
            f"local_phi={local_phi} is out of range for values={values}"
        )

    return values[local_phi]


def station_phi_to_local_phi(station_name, region, station_phi):
    if station_name == "T4E":
        return station_phi - 1

    if region == "E":
        return (station_phi - 1) % 2

    if region == "F":
        return 0

    raise ValueError(f"Unsupported region: {region}")


def BW_cell_address(region, local_phi, bw):
    station_name = bw["station_name"]
    ASDName = bw["ASDName"]
    gap = bw["gap"]

    layer = BW_global_layer(station_name, gap)

    kind = ASDName[:2]
    asd = int(ASDName[2:])

    if kind[0] != region:
        raise ValueError(f"region mismatch: region={region}, ASDName={ASDName}")

    fpga_values = station_configs[station_name]["fpga_map"][kind][asd][layer]
    ppasic_values = station_configs[station_name]["ppasic_map"][kind][asd][layer]

    fpga = select_by_local_phi(fpga_values, local_phi)
    ppASIC = select_by_local_phi(ppasic_values, local_phi)

    if ppASIC < 0 or ppASIC > 15:
        raise ValueError(f"ppASIC out of range 0..15: {ppASIC}")

    link = 1 if ppASIC < 8 else 2
    lower3 = ppASIC % 8

    upper6 = FPGA_LINK_NUMBERING[(fpga, link)]

    return (upper6 << 3) | lower3


def EIL4_cell_address(gap, isStrip, wasd):
    cell_address = 0

    if not isStrip:
        addr = [[1, 3], [0, 2], [5, 7]]
    else:
        addr = [[9, 11], [8, 10], [13, 15]]

    ppASIC = addr[gap - 1][wasd]

    link = 1 if ppASIC < 8 else 2
    lower3 = ppASIC % 8

    cell_address = (FPGA_LINK_NUMBERING[("EI-1*", link)] << 3) | lower3

    return [cell_address, ((FPGA_LINK_NUMBERING[("EI-2*", link)] << 3) | lower3)]


def is_strip_reversed(side, is_strip_reversed_aside):
    reversed = True

    if side == 1:
        reversed = True if (is_strip_reversed_aside == "backward") else False
    else:
        reversed = True if (is_strip_reversed_aside == "forward") else False

    return reversed


############################################

for station_name, cfg in station_configs.items():
    print("Station:", station_name)

    station_entry = {
        "StationName": station_name,
        "slid": [],
        "CellAddressMap": []
    }

    for side in range(2):
        for station_phi in cfg["phi_range"]:
            slid = calculate_slid(station_name, side, station_phi)

            station_entry["slid"].append({
                "side": side,
                "stationPhi": station_phi,
                "slid": slid
            })

    region = "E" if station_name.endswith("E") else "F"

    for side in range(2):

        # Wire channel map
        for gap in cfg["gap_range"][0]:
            for station_phi in cfg["phi_range"]:
                local_phi = station_phi_to_local_phi(
                    station_name,
                    region,
                    station_phi
                )

                nasd = 1

                for eta in cfg["eta_range"]:
                    for wasd in cfg["wasd_range"][eta]:

                        ASDName = (
                            f"{region}W{wasd}{EIL4_ASIDE_CHAMBERS[local_phi]['type'][0]}"
                            if station_name == "T4E"
                            else f"{region}W{wasd}"
                        )

                        if station_name == "T4E":
                            cell_address = EIL4_cell_address(gap, False, wasd)
                        else:
                            cell_address = BW_cell_address(
                                region,
                                local_phi,
                                {
                                    "station_name": station_name,
                                    "ASDName": ASDName,
                                    "gap": gap
                                }
                            )

                        channel_range = endcap_wire_channel_in_asd[station_name][ASDName][gap - 1]

                        station_entry["CellAddressMap"].append({
                            "stationEta": (eta + 1) if side == 1 else -1 * (eta + 1),
                            "stationPhi": station_phi,
                            "GasGap": gap,
                            "isStrip": 0,
                            "ASDstartChannel": nasd,
                            "channelRangeInASD": [
                                channel_range[0] + 1,
                                channel_range[1] + 1
                            ],
                            "reversed": False,
                            "cellAddress": cell_address
                        })

                        nasd += channel_range[1] - channel_range[0] + 1

        # Strip channel map
        for gap in cfg["gap_range"][1]:
            for station_phi in cfg["phi_range"]:
                local_phi = station_phi_to_local_phi(
                    station_name,
                    region,
                    station_phi
                )

                nasd = 1

                for eta in cfg["eta_range"]:
                    for sasd in range(2):

                        direction = "forward"

                        if station_name == "T4E":
                            direction = EIL4_ASIDE_CHAMBERS[local_phi]["direction"]

                        if station_name == "T4E":
                            cell_address = EIL4_cell_address(gap, True, sasd)
                        else:
                            ASDName = f"{region}S{eta * 2 + sasd}"

                            cell_address = BW_cell_address(
                                region,
                                local_phi,
                                {
                                    "station_name": station_name,
                                    "ASDName": ASDName,
                                    "gap": gap
                                }
                            )

                        station_entry["CellAddressMap"].append({
                            "stationEta": (eta + 1) if side == 1 else -1 * (eta + 1),
                            "stationPhi": station_phi,
                            "GasGap": gap,
                            "isStrip": 1,
                            "ASDstartChannel": nasd,
                            "channelRangeInASD": [1, 16],
                            "reversed": is_strip_reversed(side, direction),
                            "cellAddress": cell_address
                        })

                        nasd += 16

    cabling_data["CablingData"].append(station_entry)

with open("tgcR4Mapping.json", "w") as f:
    json.dump(cabling_data, f, indent=4)
