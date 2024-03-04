# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ..lib import DCSC_DefectTranslate_Subdetector, DCSC_Variable
from DQUtils                 import Databases
from DQUtils.channel_mapping  import get_channel_ids_names

folder, database = "/STG/DCS/HV", "COOLOFL_DCS/CONDBR2"

ids, names, _ = get_channel_ids_names(Databases.get_folder(folder, database))

STGBA, STGBC, STGEA, STGEC = 1, 2, 3, 4

def STG_HV_state(iov):
    return iov.fsmCurrentState == "ON"

class STG(DCSC_DefectTranslate_Subdetector):

    folder_base = "/STG/DCS"

    mapping = {
        STGEA: list(range(504, 1015)) +list(range(1024,1025)),
        STGEC: list(range(0,504))+list(range(1015,1024))
    }

    variables = [
        DCSC_Variable("HV", STG_HV_state),
    ]

    # If you change this please consult with the Muon groups.
    # It was decided to make it the same across CSC, MDT, RPC and TGC.
    dead_fraction_caution = None
    dead_fraction_bad = 0.1

    def __init__(self, *args, **kwargs):

        super(STG, self).__init__(*args, **kwargs)

        self.translators = [STG.color_to_defect_translator(flag, defect)
                            for flag, defect in ((STGEA, 'MS_STG_EA_STANDBY_HV'),
                                                 (STGEC, 'MS_STG_EC_STANDBY_HV'),
                                                 )]
