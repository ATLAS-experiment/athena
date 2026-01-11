# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from typing import Any
from TriggerMenuMT.L1.Base.Boards import BoardType
from ..Base.L1MenuFlags import L1MenuFlags

def defineInputsMenu():

    #-----------------------------------
    # SLOT 9 / CON 1 (CTPCal, NIM1,NIM2)
    # https://twiki.cern.ch/twiki/bin/view/Atlas/LevelOneCentralTriggerSetup#CTPIN_Slot_9
    #-----------------------------------

    ctpinBoard: dict[str,Any] = {"Ctpin9" : { "type": BoardType.CTPIN }}
    muctpiBoard: dict[str,Any] = {"MuCTPi": { "type": BoardType.MUCTPI }}
    globalBoard: dict[str,Any] = {"L0Global": { "type": BoardType.L0GLOBAL }}
    L1MenuFlags.boards().clear()
    L1MenuFlags.boards().update(ctpinBoard)
    L1MenuFlags.boards().update(muctpiBoard)
    L1MenuFlags.boards().update(globalBoard)


    # Ctpin/Slot9 (CTPCAL, NIM1, NIM2)
    ctpinBoard["Ctpin9"]["connectors"] = [
        {
            "name" : "CTPCAL",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'BCM_AtoC', 'BCM_CtoA', 'BCM_Wide', # 3 x 1-bit BCM
                ('BCM_Comb',3), # 1x 3-bit BCM
                'BCM6', 'BCM7', 'BCM8', # 2-hit BCM, for Run 3. 8 is not used
                (None, 6),
                'BMA0', 'BMA1',  # 2x BMA demonstrator
                'BPTX0','BPTX1', # 2x BPTX
                'LUCID_A', 'LUCID_C', # 2x LUCID 
                (None,4),
                'ZDC_0', 'ZDC_1', 'ZDC_2', # 3x ZDC
                'CAL0','CAL1','CAL2', # 3 x CALREQ
            ]
        },
        {
            "name" : "NIM1",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'MBTS_A0', 'MBTS_A1', 'MBTS_A2', 'MBTS_A3', 'MBTS_A4'  , 'MBTS_A5', 'MBTS_A6', 'MBTS_A7', 'MBTS_A8', 'MBTS_A9', 'MBTS_A10', 'MBTS_A11',
                'MBTS_A12', 'MBTS_A13', 'MBTS_A14', 'MBTS_A15', # 16x MBTSSI 
                ('MBTS_A',3),         # 1x MBTS_A
                'NIML1A',             # L1A for CTP monitoring itself
                'NIMLHCF',            # LHCF
                'AFP_NSA', 'AFP_FSA', 'AFP_FSA_TOF_T0', 'AFP_FSA_TOF_T1', 'AFP_FSA_TOF_T2', 'AFP_FSA_TOF_T3',   # 2xAFP
                'BMA2', 'BMA3',  # 2x BMA demonstrator
            ]
        },
        {
            "name" : "NIM2",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'MBTS_C0', 'MBTS_C1', 'MBTS_C2', 'MBTS_C3', 'MBTS_C4', 'MBTS_C5', 'MBTS_C6', 'MBTS_C7', 'MBTS_C8', 'MBTS_C9', 'MBTS_C10', 'MBTS_C11', 
                'MBTS_C12', 'MBTS_C13', 'MBTS_C14', 'MBTS_C15', # 16x MBTSSI 
                ('MBTS_C',3), # 1x MBTS_C
                'NIMTGC',     # TGC
                'NIMRPC',     # RPC
                'NIMTRT',     # TRT
                'AFP_NSC', 'AFP_FSC', 'AFP_FSC_TOF_T0', 'AFP_FSC_TOF_T1', 'AFP_FSC_TOF_T2', 'AFP_FSC_TOF_T3',   # 2xAFP
                'ZDC_ALT_0', 'ZDC_ALT_1', 'ZDC_ALT_2' # 3xZDC alternative LUCROD
            ]
        }
    ]

    # MuCTPi
    muctpiBoard["MuCTPi"]["connectors"] = [{
        "name" : "MuCTPiOpt0",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "legacy" : False,
        "thresholds" : [
            ('MU3V',3), ('MU3VF',3), ('MU3VC',3), ('MU3EOF',3), ('MU5VF',3), 
            'MU8F', 'MU8VF', 'MU8FC', 'MU8FH', 'MU8VFC', 'MU9VF', 'MU9VFC', 'MU12FCH', 
            'MU14FCH', 'MU14FCHR', 'MU15VFCH', 'MU15VFCHR', 'MU18VFCH', 'MU20VFC',
            'MU4BO', 'MU4BOM', 'MU10BO', 'MU10BOM', 'MU12BOM',
            'MU8EOF', 'MU14EOF', 
            # 57 bits for standard muon thresholds
            (None,7),
            # 64th bit for NSW monitoring
            ('NSWMon', 1)
        ]

    }]

    # Global board
    globalBoard["L0Global"]["connectors"] = [
        {
            "name": "L0Global",
            "type": "GLOBAL",
            "nbitsDefault" : 4,
            "format": "multiplicity",
            "thresholds" : [
                # 11x eEM thresholds
                ("eEM5",4), "eEM7", "eEM9", "eEM10L", "eEM12L", "eEM15", "eEM18", "eEM18L", "eEM18M", "eEM22M", "eEM24L",

                # 9x eTAU thresholds
                # "eTau1", "eTau2", "eTau12", "eTau24M", "eTau26", "eTau26L", "eTau26M", "eTau26T", "eTau140",
                
                # 6x WTACone thresholds (including 2 spare slots)
                # "WTACone100", "WTACone130", "WTACone160", "WTACone200p0Eta32C", "WTACone_Spare_Slot4", "WTACone_Spare_Slot5",
                
                # 4x eEM + WTACone thresholds with deltaEta requirement
                # "0DETA08-eEM22M-WTACone100", "08DETA16-eEM22M-WTACone100", "16DETA24-eEM22M-WTACone100", "24DETA32-eEM22M-WTACone100"
            ]
        }
    ]

