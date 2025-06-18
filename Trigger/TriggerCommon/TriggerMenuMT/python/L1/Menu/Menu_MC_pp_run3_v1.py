# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_Physics_pp_run3_v1 as physics_menu

def defineMenu():
    physics_menu.defineMenu()

    # Add new items to the item list in the Physics menu
    l1items = L1MenuFlags.items()
    l1items += [

        # ATR-28612 - legacy EM        
        'L1_EM3',
        'L1_EM18VHI',
        'L1_EM10VH',
        'L1_EM15',
        'L1_EM7_EMPTY',
        'L1_EM3_EMPTY', 
        'L1_EM7_FIRSTEMPTY',

        # ATR-28761 - Phase 1 Muon + jet
        'L1_MU14FCH_jJ80',
        'L1_MU14FCH_jXE70',
        'L1_MU14FCH_J50',
        'L1_MU14FCH_XE40',

        # ATR-19376
        'L1_MU14FCH_XE30',
        'L1_MU14FCH_J40',

        # Legacy TAU items (ATR-28677)
        'L1_TAU8_EMPTY', 'L1_TAU8_FIRSTEMPTY', 'L1_TAU8_UNPAIRED_ISO',
        'L1_TAU40_EMPTY', 'L1_TAU40_UNPAIRED_ISO',
        'L1_TAU8', 'L1_TAU12IM', 'L1_TAU20IM', 'L1_TAU40',
        'L1_TAU60', 
        'L1_TAU60_2TAU40',
        'L1_TAU100',
        'L1_DR-TAU20ITAU12I',
        'L1_DR-TAU20ITAU12I-J25',
        'L1_TAU60_DR-TAU20ITAU12I',
        'L1_TAU20IM_2TAU12IM', 
        # combined tau - jet
        'L1_TAU20IM_2TAU12IM_4J12p0ETA25',

        # ATR-24037 
        'L1_jXEPerf100',
        # ATR-22696
        'L1_eTAU60HL',
        'L1_eTAU80HL',

        # ATR-27782 - test eEM M/DR Topo
        'L1_2DR15-M70-2eEM9L',
        'L1_2DR15-M70-2eEM12L',
        'L1_2DR15-0M30-eEM12LeEM9L',
        'L1_13DR25-25M70-eEM12LeEM9L',
        
        #ATR28783
        'L1_6J15',
        # TOPO
        'L1_BPH-0M9-EM7-EM5',
        'L1_BPH-0DR3-EM7J15',
        'L1_BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25',
        'L1_BTAG-MU5VFjJ20_2jJ40p0ETA25',
        'L1_BTAG-MU3VFjJ20_2jJ40p0ETA25',
        #ATR-26902
        'L1_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20',
        'L1_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12',
        'L1_jJ85p0ETA21_3jJ40p0ETA25_cTAU20M_2cTAU12M',
        #ATR-27132
        'L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20',
        'L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12',
        'L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12',
        #ATR-29439
        'L1_cTAU30M_2cTAU20M_3jJ30p0ETA25',

        'L1_cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M',
        
        # ATR-29651 - Tau+X chains using eTAU20M seeds
        'L1_eEM18M_2eTAU20M_4jJ30', 'L1_eTAU60_2eTAU20M_jXE80', 'L1_eEM18M_2eTAU20M_jXE70', 
        'L1_MU8F_cTAU30M',

        # eTAUXXM removal
        'L1_eTAU30M_2eTAU20M_jJ55_2jJ50_3jJ30',
        'L1_MU8F_eTAU20M_jXE70',
        'L1_eTAU35M_2eTAU30M',
        'L1_MU8F_eTAU20M_jJ55_2jJ30',

        # ART-28443  test eEMX{} + {{3,4jJY{}}} L1 seeds
        'L1_eEM22M_3jJ40p0ETA25',
        'L1_eEM22M_4jJ30p0ETA25',

        # ATR-28692
        'L1_EM15VHI_2TAU12IM',
        'L1_MU8F_TAU12IM',  
        'L1_MU8F_TAU12IM_J25_2J12',
        'L1_MU8F_TAU12IM_3J12',
        'L1_EM15VHI_2TAU12IM_J25_3J12',
        'L1_MU8F_TAU20IM',
        #
        'L1_TAU20IM_2TAU12IM_J25_2J20_3J12',
        'L1_TAU25IM_2TAU20IM',
        #
        'L1_TAU20IM_2J20_XE45',
        'L1_EM15VHI_2TAU12IM_XE35',
        'L1_EM15VHI_2TAU12IM_4J12',
        'L1_MU8F_TAU12IM_XE35',  
        'L1_TAU20IM_2TAU12IM_XE35', 
        'L1_TAU40_2TAU12IM_XE40',
        'L1_TAU25IM_2TAU20IM_2J25_3J20',

        #ATR-28679 - legacy XE
        'L1_XE35', 'L1_XE40', 'L1_XE45', 'L1_XE50','L1_XE55', 'L1_XE60',
        # ATR-29609
        'L1_XE30', 'L1_XE300',

        # Legacy combined em - jet moved by ATR-28761
        'L1_EM18VHI_3J20',
        'L1_EM20VH_3J20',

        # Legacy combined mu - jet moved by ATR-28761
        'L1_MU3V_J12',
        # Legacy ATR-13743 J,XE thershold change for ATR-19376  moved by ATR-28761
        'L1_MU8F_2J20','L1_MU8F_3J20', 'L1_MU8F_2J15_J20',
        'L1_3J15p0ETA25_XE40',
        'L1_2J15_XE55',
        'L1_J40_XE50',
        'L1_2J50_XE40',
        'L1_J40_XE60',
        'L1_AFP_A_AND_C_TOF_J20', 'L1_AFP_A_AND_C_TOF_T0T1_J20', 'L1_AFP_A_AND_C_TOF_J30', 'L1_AFP_A_AND_C_TOF_T0T1_J30', 'L1_AFP_A_AND_C_TOF_J50', 'L1_AFP_A_AND_C_TOF_T0T1_J50', 'L1_AFP_A_AND_C_TOF_J75', 'L1_AFP_A_AND_C_TOF_T0T1_J75',
        'L1_AFP_A_OR_C_J12', 'L1_AFP_A_AND_C_J12',
        
        # ATR-28678
        "L1_J12",
        "L1_J25",
        "L1_J30",
        "L1_J40",
        "L1_J75",
        "L1_J85",
        "L1_J100",
        "L1_J120",
        "L1_J400_LAR",
        "L1_J12_EMPTY",
        "L1_J30_EMPTY",
        "L1_J30_FIRSTEMPTY",
        "L1_J45p0ETA21_3J15p0ETA25",
        "L1_4J20",
        "L1_J85_3J30",
        "L1_J25p0ETA23_2J15p31ETA49",
        "L1_J40p0ETA25_2J25_J20p31ETA49",
        "L1_3J35p0ETA23",
        "L1_4J15p0ETA25",
        "L1_5J15p0ETA25",
        'L1_J30p31ETA49_EMPTY',


        # ATR-29303
        'L1_J15',
        'L1_J20',
        'L1_J50',
        'L1_J400',
        'L1_J75p31ETA49',
        'L1_J20p31ETA49',
        'L1_J30p31ETA49',
        'L1_J50p31ETA49',
        'L1_J15p31ETA49',
        'L1_MU3V_J15',
        'L1_MU5VF_J40',
        'L1_J50_2J40p0ETA25_3J15p0ETA25',
        'L1_3J50',
        'L1_J40p0ETA25_2J15p31ETA49',
        'L1_3J25p0ETA23',

        # legacy L1Topo
        'L1_HT190-J15s5pETA21', 
        'L1_BPH-0M9-EM7-EM5_2MU3V',
        'L1_BPH-0M9-EM7-EM5_MU5VF',
        'L1_BPH-0DR3-EM7J15_2MU3V',
        'L1_BPH-0DR3-EM7J15_MU5VF',
        'L1_JPSI-1M5-EM7',
        'L1_JPSI-1M5-EM12',
        'L1_MJJ-500-NFF',
        'L1_MJJ-700',
        'L1_EM18VHI_MJJ-300',
        'L1_HT150-J20s5pETA31_MJJ-400-CF',
        'L1_LLP-RO',
        'L1_LLP-NOMATCH',
        'L1_SC111-CJ15',
        'L1_J50_DETA20-J50J',
        'L1_LAR-ZEE',

        #ATR-29330
        'L1_4J15',

    ]

    # To replace thresholds in the physics menu
    # Do not use for L1Topo decision threshold!
    L1MenuFlags.ThresholdMap = {
        #example: 'jXE100' :'',
    }

    # To replace items in the physics menu
    L1MenuFlags.ItemMap = {


        # non-primary eEM
        'L1_eEM7':'',
        'L1_eEM10L':'',
        'L1_eEM15':'',
        #'L1_eEM18':'',
        'L1_eEM22M':'',
        #'L1_eEM24VM':'',
        'L1_3eEM12L':'',

        # non-primary TAU
        'L1_eTAU35':'',
        'L1_eTAU40HM':'',
        'L1_2TAU8':'',
        'L1_eEM18M_2eTAU20M':'',
        #'L1_MU8F_eTAU20M':'',
        'L1_MU8F_eTAU20M_jJ55_2jJ30':'',
        'L1_eEM18M_2eTAU20M_jJ55_3jJ30':'',
        'L1_eTAU30M_2eTAU20M_jJ55_2jJ50_3jJ3':'',
        'L1_eTAU30M_2jJ50_jXE90':'',
        'L1_eTAU30M_2eTAU20M_jXE70':'',

        # non-primary MU 
        'L1_MU14FCHR':'',
        'L1_MU8FC':'', 
        'L1_MU15VFCH':'',
        'L1_2MU8VF':'',
        'L1_2MU14FCH_OVERLAY':'',
        'L1_MU3VC':'',
        'L1_MU4BO':'',
        'L1_MU3EOF':'',
        'L1_MU8FH':'',
        'L1_MU8EOF':'',
        'L1_MU9VF':'',
        'L1_MU9VFC':'',
        'L1_MU14EOF':'',
        'L1_MU15VFCHR':'',
        'L1_MU20VFC':'',

        # non-primary J
        'L1_J12':'',
        'L1_J12_BGRP12':'',
        'L1_jJ30p0ETA25':'',
        'L1_jJ40p0ETA25':'',
        'L1_jJ55':'',
        'L1_jJ55p0ETA23':'',
        'L1_jJ70p0ETA23':'',
        'L1_jJ80p0ETA25':'',
        'L1_jJ85p0ETA21':'',
        'L1_jJ140':'',

        # other non-primary
        'L1_jEM25':'',
        'L1_jEM20M':'',

        # combined non-primary
        'L1_MU8F_2J20':'',
        'L1_MU8F_3J20':'',

        # MU non-FILLED
        'L1_MU3V_FIRSTEMPTY':'', 
        'L1_MU8VF_EMPTY':'',

        # EM non-FILLED

        # J non-FILLED
        'L1_J12_FIRSTEMPTY':'', 
        'L1_J12_UNPAIRED_ISO':'', 
        'L1_J12_UNPAIRED_NONISO':'', 
        'L1_J12_UNPAIREDB1':'', 
        'L1_J12_UNPAIREDB2':'', 
        'L1_J15p31ETA49_UNPAIRED_ISO':'',
        'L1_J30p31ETA49_EMPTY':'', 
        'L1_J30p31ETA49_UNPAIRED_ISO':'', 
        'L1_J30p31ETA49_UNPAIRED_NONISO':'',
        'L1_J50_UNPAIRED_ISO':'', 
        'L1_J50_UNPAIRED_NONISO':'', 
        'L1_J100_FIRSTEMPTY':'',

        # Others
        'L1_J400_LAR':'',
        'L1_jJ500_LAR':'',

        'L1_TRT_EMPTY':'',
        'L1_TRT_FILLED':'',

        'L1_RD0_UNPAIRED_ISO':'',
        'L1_RD0_FIRSTINTRAIN':'',
        'L1_RD0_FIRSTEMPTY':'', 
        'L1_RD0_BGRP11':'',
        'L1_RD0_BGRP7':'',
        'L1_RD1_EMPTY':'',
        'L1_RD2_EMPTY':'',
        'L1_RD2_FILLED':'',
        'L1_RD3_EMPTY':'',
        'L1_RD3_FILLED':'',

        'L1_TGC_BURST':'',

        'L1_LUCID_A':'', 
        'L1_LUCID_C':'',

        'L1_BPTX0_BGRP12':'',
        'L1_BPTX1_BGRP12':'',

        'L1_CALREQ0':'',
        'L1_CALREQ1':'',
        'L1_CALREQ2':'',

        'L1_MBTS_A':'',
        'L1_MBTS_C':'',
        'L1_MBTS_1_EMPTY':'',
        'L1_MBTS_1_1_EMPTY':'',
        'L1_MBTS_2_EMPTY':'',
        'L1_MBTS_1_UNPAIRED_ISO':'',
        'L1_MBTS_1_1_UNPAIRED_ISO':'',
        'L1_MBTS_2_UNPAIRED_ISO':'',
        'L1_MBTS_1':'',
        'L1_MBTS_1_1':'',
        'L1_MBTS_2':'',
        'L1_MBTS_4_A':'',
        'L1_MBTS_4_C':'',
        'L1_MBTS_1_A':'',
        'L1_MBTS_1_C':'',
        'L1_MBTS_1_A_EMPTY':'',
        'L1_MBTS_1_C_EMPTY':'',

        'L1_MBTSA0':'',
        'L1_MBTSA1':'',
        'L1_MBTSA2':'',
        'L1_MBTSA3':'',
        'L1_MBTSA4':'',
        'L1_MBTSA5':'',
        'L1_MBTSA6':'',
        'L1_MBTSA7':'',
        'L1_MBTSA8':'',
        'L1_MBTSA9':'',
        'L1_MBTSA10':'',
        'L1_MBTSA11':'',
        'L1_MBTSA12':'',
        'L1_MBTSA13':'',
        'L1_MBTSA14':'', 
        'L1_MBTSA15':'',
        'L1_MBTSC0':'', 
        'L1_MBTSC1':'',
        'L1_MBTSC2':'', 
        'L1_MBTSC3':'',
        'L1_MBTSC4':'',
        'L1_MBTSC5':'', 
        'L1_MBTSC6':'',
        'L1_MBTSC7':'', 
        'L1_MBTSC8':'',
        'L1_MBTSC9':'', 
        'L1_MBTSC10':'', 
        'L1_MBTSC11':'', 
        'L1_MBTSC12':'', 
        'L1_MBTSC13':'', 
        'L1_MBTSC14':'', 
        'L1_MBTSC15':'', 

        'L1_BCM_Wide_BGRP12':'', 
        'L1_BCM_2A_2C_UNPAIRED_ISO':'',
        'L1_BCM_2A_2C_BGRP12':'',
        'L1_BCM_Wide_EMPTY':'', 
        'L1_BCM_Wide':'',
        'L1_BCM_Wide_CALIB':'',
        'L1_BCM_Wide_UNPAIREDB1':'', 
        'L1_BCM_Wide_UNPAIREDB2':'',
        'L1_J12_UNPAIREDB1':'', 
        'L1_J12_UNPAIREDB2':'',
        'L1_BCM_2A_EMPTY':'',
        'L1_BCM_2C_EMPTY':'',
        'L1_BCM_2A_UNPAIREDB1':'',
        'L1_BCM_2C_UNPAIREDB1':'',
        'L1_BCM_2A_UNPAIREDB2':'',
        'L1_BCM_2C_UNPAIREDB2':'',
        'L1_BCM_2A_FIRSTINTRAIN':'',
        'L1_BCM_2C_FIRSTINTRAIN':'',
        'L1_BCM_2A_CALIB':'',
        'L1_BCM_2C_CALIB':'',

        'L1_AFP_A_OR_C_UNPAIRED_ISO':'',
        'L1_AFP_A_OR_C_UNPAIRED_NONISO':'',
        'L1_AFP_A_OR_C_EMPTY':'',
        'L1_AFP_A_OR_C_FIRSTEMPTY':'',
        'L1_AFP_FSA_BGRP12':'',
        'L1_AFP_FSC_BGRP12':'',
        'L1_AFP_NSA_BGRP12':'',
        'L1_AFP_NSC_BGRP12':'',
        'L1_AFP_A':'',
        'L1_AFP_C':'',
        'L1_AFP_A_AND_C_TOF_T0T1':'',
        'L1_AFP_FSA_TOF_T0_BGRP12':'',
        'L1_AFP_FSA_TOF_T1_BGRP12':'',
        'L1_AFP_FSC_TOF_T0_BGRP12':'',
        'L1_AFP_FSC_TOF_T1_BGRP12':'',
        'L1_AFP_FSA_TOF_T2_BGRP12':'',
        'L1_AFP_FSA_TOF_T3_BGRP12':'',
        'L1_AFP_FSC_TOF_T2_BGRP12':'',
        'L1_AFP_FSC_TOF_T3_BGRP12':'',
    } 

    #----------------------------------------------
    def remapItems():  
        itemsToRemove = []
        for itemIndex, itemName in enumerate(L1MenuFlags.items()):
            if itemName in L1MenuFlags.ItemMap():
                if L1MenuFlags.ItemMap()[itemName] != '':
                    L1MenuFlags.items()[itemIndex] = L1MenuFlags.ItemMap()[itemName]                                                
                else: 
                    itemsToRemove.append(itemIndex)

        for i in reversed(itemsToRemove):
            del L1MenuFlags.items()[i]
    #----------------------------------------------
                                           
    remapItems()

