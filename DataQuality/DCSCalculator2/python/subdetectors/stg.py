# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ..lib import DCSC_DefectTranslate_Subdetector, DCSC_Variable
from DQUtils                 import Databases
from DQUtils.channel_mapping  import get_channel_ids_names

folder, database = "/STG/DCS/HV", "COOLOFL_DCS/CONDBR2"

ids, names, _ = get_channel_ids_names(Databases.get_folder(folder, database))

STGBA, STGBC, STGEA, STGEC, STGIGNORE = 1, 2, 3, 4, 5

#list of channel IDs to ignore (correspond to R1 chambers that have problems with HV) 
REMOVE_FROM_STGEA = {505, 506, 507, 508, 512, 514, 521, 525, 528, 532,              \
    536, 540, 546, 553, 560, 567, 571, 575, 579, 583, 586, 590, 594, 598, 602, 606, 610, 614, 617, 621, 625, 629, 633, 637, 641, 645, 649, 653, 657, 660, 
663, 667, \
    671, 675, 679, 683, 687, 691, 695, 699, 703, 707, 711, 715, 719, 723, 727, 731, \
    735, 739, 743, 747, 755, 759, 763, 767, 771, 775, 779, 783, 787, 791, 795, 799, \
    803, 807, 811, 815, 819, 823, 827, 831, 835, 839, 843, 847, 851, 855, 859, 863, \
    867, 871, 875, 879, 883, 887, 891, 895, 899, 903, 907, 911, 915, 919, 923, 927, \
    931, 935, 939, 943, 947, 951, 955, 959, 963, 967, 971, 975, 979, 983, 987, 991, \
    995, 999, 1003, 1007, 1011} 

REMOVE_FROM_STGEC = {1, 5, 9, 13, 17, 20, 24, 28, 32, 36, 39, 43, 46, 50, 54, 58, 62, 65,\
    69, 73, 77, 81, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,\
    128, 132, 136, 140, 144, 147, 151, 155, 159, 163, 167, 171, 174,\
    177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221, 225,\
    229, 233, 237, 241, 245, 249, 252, 256, 260, 264, 268, 272, 276,\
    280, 284, 288, 292, 296, 300, 304, 308, 312, 316, 320, 324, 328,\
    332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,\
    384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432,\
    436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484,\
    488, 492, 496, 500}


def STG_HV_state(iov):
    return iov.fsmCurrentState == "ON"

class STG(DCSC_DefectTranslate_Subdetector):

    folder_base = "/STG/DCS"

    mapping = {
#        STGEA: list(range(504, 1015)) +list(range(1024,1025)),
#        STGEC: list(range(0,504))+list(range(1015,1024))
#    }

       STGEA: [
           y for y in (list(range(504, 1015)) + list(range(1024, 1025)))
           if y not in REMOVE_FROM_STGEA
    ],
       STGEC: [
           x for x in (list(range(0, 504)) + list(range(1015, 1024)))
           if x not in REMOVE_FROM_STGEC
    ],  
       STGIGNORE: list(REMOVE_FROM_STGEA) + list(REMOVE_FROM_STGEC)
    }
      

    variables = [
        DCSC_Variable("HV", STG_HV_state),
    ]

    # If you change this please consult with the Muon groups.
    # It was decided to make it the same across CSC, MDT, RPC and TGC.
    dead_fraction_caution = None
    dead_fraction_bad = 0.2

    def __init__(self, *args, **kwargs):

        super(STG, self).__init__(*args, **kwargs)

        self.translators = [STG.color_to_defect_translator(flag, defect)
                            for flag, defect in ((STGEA, 'MS_STG_EA_STANDBY_HV'),
                                                 (STGEC, 'MS_STG_EC_STANDBY_HV'),
                                                 )]
