# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ..lib import DCSC_DefectTranslate_Subdetector, DCSC_Variable
from DQUtils                 import Databases
from DQUtils.channel_mapping  import get_channel_ids_names
folder, database = "/MMG/DCS/HV", "COOLOFL_DCS/CONDBR2"

ids, names, _ = get_channel_ids_names(Databases.get_folder(folder, database))
    

MMGE1A,MMGE1C=1,2

def MMG_HV_state(iov):
    return iov.fsmCurrentState == "ON"

class MMG(DCSC_DefectTranslate_Subdetector):

    folder_base = "/MMG/DCS"
    
    #Plan in the future is to have different conditions for the drift and not chambers so for the moment the A and C sectors/
    #contain both types of chambers with the ranges split into either fully drift or non drift ranges for easier future changes/
    #as drift chambers are interspersed between non drift chambers.

    mapping = {
        MMGE1A: list(range(5, 45))+list(range(48, 68))+list(range(72, 112))+list(range(116, 156))+list(range(160, 200))+
        list(range(204, 244))+list(range(248, 288))+list(range(292, 332))+list(range(336, 376))+list(range(380, 420))+
        list(range(424, 464))+list(range(468, 508))+list(range(512, 552))+list(range(556, 596))+list(range(600,639 
        ))+list(range(643,683))+list(range(867,868))+list(range(880,881))+list(range(882,883))+list(range(887,888))
        +list(range(908,909))+list(range(912,913))+list(range(915, 938))+list(range(939, 1315))+list(range(1, 5))+
        list(range(45, 48))+list(range(68, 72))+list(range(112, 116))+list(range(156, 160))+list(range(200, 204))+
        list(range(244, 248))+list(range(288, 292))+list(range(332, 336))+list(range(376, 380))+list(range(420, 424))+
        list(range(464, 468))+list(range(508, 512))+list(range(552, 556))+list(range(596,600 ))+list(range(639, 643))+
        list(range(938, 939)),
        
        
        MMGE1C: list(range(684, 725))+list(range(726, 746))+list(range(747, 767))+list(range(768, 788))+list(range(789, 809))+
        list(range(811, 867))+list(range(868, 880))+list(range(881, 882))+list(range(883, 887))+list(range(888, 908))+
        list(range(909, 912))+list(range(913, 915))+list(range(1318, 1362))+list(range(1365, 1409))+list(range(1412,1454 ))+
        list(range(1457, 1500))+list(range(1503, 1545))+list(range(1548, 1590))+list(range(1594, 1650))+list(range(1654, 1716))+
        list(range(1720, 1780))+list(range(1782, 1796))+list(range(1800, 1856))+list(range(1860, 1915))+list(range(1919,1978 ))+
        list(range(1982, 2043))+list(range(2047, 2110))+list(range(2114, 2177))+list(range(683, 684))+list(range(725,726))+
        list(range(746,747))+list(range(767,768))+list(range(788,789))+list(range(809,810))+list(range(810,811))+
        list(range(1315, 1318))+list(range(1362, 1365))+list(range(1409, 1412))+list(range(1454, 1457))+list(range(1500, 1503))+
        list(range(1545, 1548))+list(range(1590, 1594))+list(range(1650, 1654))+list(range(1716, 1720))+list(range(1780, 1782))+
        list(range(1796, 1800))+list(range(1856, 1860))+list(range(1915, 1919))+list(range(1978,1982 ))+list(range(2043, 2047))+
        list(range(2110, 2114))
    }
    

    variables = [
        DCSC_Variable("HV", MMG_HV_state),
    ]

    # If you change this please consult with the Muon groups.
    # It was decided to make it the same across CSC, MDT, RPC and TGC.
    dead_fraction_caution = None
    dead_fraction_bad = 0.1

    def __init__(self, *args, **kwargs):

        super(MMG, self).__init__(*args, **kwargs)

        self.translators = [MMG.color_to_defect_translator(flag, defect)
                            for flag, defect in ((MMGE1A, 'MS_MMG_EA_STANDBY_HV'),
                                                 (MMGE1C, 'MS_MMG_EC_STANDBY_HV')
                                                 )]
