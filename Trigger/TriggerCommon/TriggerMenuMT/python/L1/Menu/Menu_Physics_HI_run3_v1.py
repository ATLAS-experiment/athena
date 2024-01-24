# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Menu.MenuCommon import print_available, RequiredL1Items, FixedIDMap, defineCommonL1Flags

def defineMenu():

    defineCommonL1Flags(L1MenuFlags)

    L1MenuFlags.items = RequiredL1Items + [

        ##
        # single EM
        ##
        #'L1_EM8VH',
        'L1_EM10',
        'L1_EM10VH',
        'L1_EM12',
        #'L1_EM14', 
        'L1_EM15', 
        #'L1_EM16','L1_EM18VH', 
        #'L1_EM20VHI', 'L1_EM22',
        #'L1_EM22VHI',
        #'L1_EM20VH_FIRSTEMPTY',
        # new calo
        'L1_eEM1', 'L1_eEM2',
        'L1_eEM5', 'L1_eEM9', 'L1_eEM12', 'L1_eEM18', 'L1_eEM15',
        'L1_eEM12L', 'L1_eEM18L', 'L1_eEM26', 'L1_eEM26M',
        # ATR-22061
        "L1_eEM9_EMPTY",
        "L1_eEM15_EMPTY",

        ## 
        # MU
        ##
        'L1_MU3V', 'L1_MU5VF', 'L1_MU8F', 'L1_MU8VF', 'L1_MU14FCH',
        'L1_2MU3V', 'L1_2MU5VF', 'L1_2MU8F',
        'L1_3MU3V',

        'L1_2MU14FCH_OVERLAY',
        'L1_MU3V_EMPTY', 'L1_2MU5VF_EMPTY', 'L1_MU3V_FIRSTEMPTY', 'L1_MU8VF_EMPTY',
        'L1_MU3V_UNPAIRED_ISO',

        ##
        # combined lepton (e and mu)
        ##
        'L1_2EM15',
        #'L1_2EM10', 'L1_2EM16',
        #'L1_2EM20VH',
        # new calo
        #'L1_2eEM7', 'L1_2eEM9', 'L1_2eEM15',
        'L1_2eEM12', 'L1_2eEM18',

        
        # combined mu - jet
        'L1_MU3V_J12',
        'L1_MU3V_J20',
        'L1_MU3V_J30',
        'L1_MU3V_jJ40',

        'L1_TAU8', 'L1_TAU12IM', 'L1_TAU20IM', 'L1_eTAU12_EMPTY', 'L1_eTAU80', 

        # single jet
        'L1_J12','L1_J15','L1_J20','L1_J25', 'L1_J30', 'L1_J40', 'L1_J50' ,'L1_J75','L1_J85', 'L1_J100',
        'L1_J20p31ETA49', 'L1_J30p31ETA49', 'L1_J50p31ETA49', 'L1_J75p31ETA49', 'L1_J15p31ETA49',
        'L1_J12_EMPTY','L1_J12_FIRSTEMPTY', 'L1_J12_UNPAIRED_ISO', 'L1_J12_UNPAIRED_NONISO', 'L1_J12_UNPAIREDB1', 'L1_J12_UNPAIREDB2',
        'L1_J15p31ETA49_UNPAIRED_ISO',
        'L1_J30_EMPTY', 'L1_J30_FIRSTEMPTY', 'L1_J30p31ETA49_EMPTY', 'L1_J30p31ETA49_UNPAIRED_ISO', 'L1_J30p31ETA49_UNPAIRED_NONISO',
        'L1_J50_UNPAIRED_ISO', 'L1_J50_UNPAIRED_NONISO',
        'L1_J100_FIRSTEMPTY',
        'L1_J12_BGRP12',
        'L1_J400', 'L1_J400_LAR',
        # new calo
        'L1_jJ500', 'L1_jJ500_LAR',
        'L1_jJ20', 'L1_jJ30',
        'L1_jJ40', 'L1_jJ50', 'L1_jJ55', 'L1_jJ60', 'L1_jJ80', 'L1_jJ90',
        'L1_jJ15p30ETA49', 'L1_jJ20p30ETA49',
        'L1_jJ40p30ETA49', 'L1_jJ50p30ETA49', 'L1_jJ60p30ETA49', 'L1_jJ90p30ETA49', 'L1_jJ125p30ETA49',

        # gJ - ATR-28029
        "L1_gJ20p0ETA25","L1_gJ400p0ETA25","L1_gLJ80p0ETA25","L1_gTE200","L1_gXEJWOJ100",

        # XE
        'L1_XE35', 'L1_XE40', 'L1_XE45', 'L1_XE50', 
        'L1_XE55', 'L1_XE60', 'L1_XE30', 'L1_XE300',
       
        'L1_J40_XE50', 'L1_J40_XE60', 
 
         # calo
        'L1_TE3', 'L1_TE4', 'L1_TE5', # also for HMT triggers
        'L1_TE10', 'L1_TE20', 'L1_TE50',
        'L1_TE100', 'L1_TE200',
        'L1_TE3p0ETA49', 'L1_TE7p0ETA49',
        'L1_TE600p0ETA49', 'L1_TE1500p0ETA49', 'L1_TE3000p0ETA49', 'L1_TE3500p0ETA49', 'L1_TE6500p0ETA49', 'L1_TE8000p0ETA49',
        'L1_TE50_VTE600p0ETA49',
        # calo overlay
        'L1_MBTS_1_VTE50_OVERLAY',
        'L1_TE50_OVERLAY', 'L1_TE600p0ETA49_OVERLAY', 'L1_TE1500p0ETA49_OVERLAY', 'L1_TE3000p0ETA49_OVERLAY',
        'L1_TE3500p0ETA49_OVERLAY', 'L1_TE6500p0ETA49_OVERLAY', 'L1_TE8000p0ETA49_OVERLAY',
        
        # new calo
        'L1_jTE200',
        # additional jTE items for 2023 heavy ion runs
        'L1_jTE3', 'L1_jTE4', 'L1_jTE5',
        'L1_jTE10',
        'L1_jTE20','L1_jTE50',
        'L1_jTE100',
        'L1_jTE600',
        'L1_jTE1500',
        'L1_jTE3000',
        'L1_VjTE200',
        'L1_VjTE600',

        #L1 forward GAP
        'L1_GAP_A', 'L1_GAP_C', 'L1_GAP_AANDC',

        #UPC - MU
        'L1_MU3V_VTE50', 'L1_MU5VF_VTE50', 'L1_2MU3V_VTE50', 'L1_MU3V_VTE200',
        
        #UPC - MU, phase-1 calo
        'L1_MU3V_VjTE50', 'L1_MU5VF_VjTE50', 'L1_2MU3V_VjTE50',
        
        #UPC - EM
        'L1_TAU1_TE4_VTE200', 'L1_TAU1_TE5_VTE200',
        'L1_TAU2_TE4_VTE200',
        'L1_TAU1_TE4_VTE200_EMPTY',
        'L1_2TAU1_VTE200', 'L1_2TAU1_VTE200_EMPTY',
        'L1_2TAU1_VTE200_UNPAIRED_ISO', 'L1_2TAU1_VTE200_UNPAIRED_NONISO',
        'L1_TAU8_VTE200', 'L1_TAU8_VTE200_EMPTY',
        #'L1_EM7_VTE200',
        
        #UPC - new EM
        'L1_eEM1_VjTE200', 'L1_eEM2_VjTE200', 'L1_eEM5_VjTE200', 'L1_2eEM1_VjTE200', 'L1_2eEM2_VjTE200','L1_2eEM1_VjTE200_GAP_AANDC',
        'L1_2eEM1_VjTE200_EMPTY','L1_2eEM1_VjTE200_UNPAIRED_ISO','L1_2eEM1_VjTE200_UNPAIRED_NONISO',
        'L1_eEM1_TE4_VTE200', 'L1_eEM2_TE4_VTE200', 'L1_eEM1_TE4_VTE200_EMPTY',
        'L1_eEM1_VTE200', 'L1_2eEM1_VTE200', 'L1_2eEM2_VTE200', 'L1_eEM9_VTE200',
        
        #UPC - calo, MBTS, calo  
        'L1_ZDC_XOR_VTE200', 'L1_VZDC_A_VZDC_C_TE5_VTE200', 'L1_VZDC_A_VZDC_C_TE10_VTE200',
        'L1_ZDC_A_VZDC_C_VTE200', 'L1_VZDC_A_ZDC_C_VTE200',
        'L1_MBTS_1_ZDC_A_VZDC_C_VTE200', 'L1_MBTS_1_VZDC_A_ZDC_C_VTE200',
        
        #UPC - calo, MBTS - legacy
        'L1_MBTS_1_VTE50',
        'L1_MBTS_1_1_VTE50',
        'L1_MBTS_1_VTE200',
        #UPC - calo, TRT - legacy
        'L1_TRT_VTE50',
        'L1_TRT_VTE200',
        'L1_TRT_VTE20',
        'L1_TAU1_TRT_VTE50',
        #UPC - TRT,  phase-1 calo
        'L1_TRT_VjTE50',
        #UPC - calo only - legacy
        'L1_VTE20',
        'L1_VTE50', 'L1_TE3_VTE50',
        'L1_VTE200', 'L1_TE5_VTE200', 'L1_TE50_VTE200',
        'L1_J12_VTE200',
         #UPC, calo only, phase-1
         'L1_jTE5_VjTE200',

        
        #LUCID
        'L1_LUCID_A', 'L1_LUCID_C',
        'L1_LUCID_A_BGRP11', 'L1_LUCID_C_BGRP11',

        # ZDC
        'L1_ZDC_A','L1_ZDC_C','L1_ZDC_A_C',
        'L1_ZDC_XOR',
        'L1_ZDC_C_VZDC_A', 'L1_ZDC_A_VZDC_C',
        'L1_ZDC_A_EMPTY','L1_ZDC_C_EMPTY','L1_ZDC_A_C_EMPTY',
        'L1_ZDC_A_UNPAIRED_NONISO','L1_ZDC_C_UNPAIRED_NONISO','L1_ZDC_A_C_UNPAIRED_NONISO',
        'L1_ZDC_A_VTE200', 'L1_ZDC_C_VTE200',
        'L1_TRT_ZDC_A_VTE50', 'L1_TRT_ZDC_C_VTE50',

        # Run3 ZDC items for heavy ion runs 
        'L1_VZDC_A_VZDC_C', #comb0
        'L1_1ZDC_A_VZDC_C', #comb4
        'L1_VZDC_A_1ZDC_C', #comb6
        'L1_1ZDC_A_1ZDC_C', #comb1
        'L1_5ZDC_A_VZDC_C', #comb5
        'L1_VZDC_A_5ZDC_C', #comb7
        'L1_ZDC_1XOR5',     #comb2
        'L1_5ZDC_A_5ZDC_C', #comb3
        
        #ZDC and legacy calo
        'L1_1ZDC_A_1ZDC_C_VTE200', 'L1_ZDC_1XOR5_VTE200',
        'L1_MBTS_1_1ZDC_A_1ZDC_C_VTE200', 'L1_MBTS_1_ZDC_1XOR5_VTE200',
        'L1_VZDC_A_ZDC_C_TE3_VTE200', 'L1_1ZDC_A_1ZDC_C_TE3_VTE200',
        'L1_ZDC_1XOR5_TE3_VTE200', 'L1_ZDC_A_VZDC_C_TE3_VTE200',
        'L1_VZDC_A_ZDC_C_TE5_VTE200', 'L1_1ZDC_A_1ZDC_C_TE5_VTE200',
        'L1_ZDC_1XOR5_TE5_VTE200', 'L1_ZDC_A_VZDC_C_TE5_VTE200',
        'L1_MBTS_1_ZDC_XOR_VTE200', 
        'L1_ZDC_XOR_TE3_VTE200', 'L1_ZDC_XOR_TE5_VTE200',
        'L1_1ZDC_NZDC_TE5_VTE200',
        'L1_MBTS_2_VZDC_A_ZDC_C_VTE200', 'L1_MBTS_2_1ZDC_A_1ZDC_C_VTE200',
        'L1_MBTS_2_ZDC_1XOR5_VTE200', 'L1_MBTS_2_ZDC_A_VZDC_C_VTE200',
         
        'L1_ZDC_5XOR_TE5_VTE200', 'L1_ZDC_XOR4_TE5_VTE200',
        'L1_VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO', 'L1_ZDC_XOR_TE5_VTE200_UNPAIRED_ISO',
        'L1_5ZDC_A_5ZDC_C_TE5_VTE200', 'L1_VZDC_A_VZDC_C_VTE200',
        'L1_VZDC_A_VZDC_C_TE5', 'L1_ZDC_XOR_TE5',

        'L1_ZDC_A_C_VTE10', 'L1_ZDC_XOR_VTE10', 'L1_ZDC_A_C_VTE10_UNPAIRED_ISO',
        'L1_ZDC_A_C_VTE10_UNPAIRED_NONISO', 'L1_ZDC_A_C_VTE10_EMPTY',

        'L1_TAU1_VZDC_A_VZDC_C_VTE100', 'L1_TAU1_ZDC_XOR4_VTE100',
        'L1_TAU2_VZDC_A_VZDC_C_VTE100', 'L1_TAU2_ZDC_XOR4_VTE100',
        'L1_TAU1_TRT_VZDC_A_VZDC_C_VTE100', 'L1_TAU1_TRT_ZDC_XOR4_VTE100',
        'L1_TRT_VZDC_A_VZDC_C_VTE50', 'L1_TRT_VZDC_A_VZDC_C_VTE20',
        'L1_TRT_VZDC_A_VZDC_C_VTE200',

        'L1_ZDC_A_C_VTE50',
        'L1_ZDC_A_C_VTE50_OVERLAY',
        'L1_ZDC_XOR4_VTE200', 'L1_VZDC_A_VZDC_C_VTE50',
        'L1_ZDC_OR_VTE200_UNPAIRED_ISO', 'L1_MBTS_1_ZDC_OR_VTE200_UNPAIRED_ISO',

        #ZDC and phase-1 calo
        'L1_1ZDC_A_1ZDC_C_VjTE200', 'L1_ZDC_1XOR5_VjTE200',
        'L1_ZDC_XOR_VjTE200', 
        'L1_MBTS_1_VZDC_A_ZDC_C_VjTE200', 'L1_MBTS_1_1ZDC_A_1ZDC_C_VjTE200',
        'L1_MBTS_1_ZDC_1XOR5_VjTE200', 'L1_MBTS_1_ZDC_A_VZDC_C_VjTE200',
        'L1_VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A', 'L1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_A',
        'L1_ZDC_1XOR5_jTE5_VjTE200_GAP_A', 'L1_ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C',
        'L1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_C', 'L1_ZDC_1XOR5_jTE5_VjTE200_GAP_C',
        'L1_MBTS_1_ZDC_XOR_VjTE200',
        'L1_VZDC_A_ZDC_C_VjTE200', 'L1_ZDC_A_VZDC_C_VjTE200',

        'L1_VZDC_A_VZDC_C_VjTE200',


        'L1_eEM1_VZDC_A_VZDC_C_VjTE100', 'L1_eEM1_ZDC_XOR4_VjTE100',
        'L1_eEM2_VZDC_A_VZDC_C_VjTE100', 'L1_eEM2_ZDC_XOR4_VjTE100',
        'L1_eEM1_VZDC_A_VZDC_C_VTE100', 'L1_eEM1_ZDC_XOR4_VTE100',
        'L1_eEM2_VZDC_A_VZDC_C_VTE100', 'L1_eEM2_ZDC_XOR4_VTE100',
        'L1_TRT_VZDC_A_VZDC_C_VjTE50',


        'L1_ZDC_XOR4_VjTE200_GAP_AANDC',
        'L1_1ZDC_A_1ZDC_C_VjTE200_GAP_AANDC', 'L1_VZDC_A_VZDC_C_VjTE200_GAP_AANDC',
        'L1_ZDC_OR_VjTE200_UNPAIRED_ISO', 'L1_MBTS_1_ZDC_OR_VjTE200_UNPAIRED_ISO',
        

        # VDM

        # ZDC bits and comb for debugging
        'L1_ZDC_BIT2',
        'L1_ZDC_BIT1',
        'L1_ZDC_BIT0',
        'L1_ZDC_COMB0',
        'L1_ZDC_COMB1',
        'L1_ZDC_COMB2',
        'L1_ZDC_COMB3',
        'L1_ZDC_COMB4',
        'L1_ZDC_COMB5',
        'L1_ZDC_COMB6',
        'L1_ZDC_COMB7',


        # ZDC items for LHCf+ZDC special run ATR-26051
        # Commented out for more CTP space for 2022 Nov heavy ion test run (ATR-26405) 
        # They are needed for scheduled 2023 5 TeV pp runs, so not removed from the menu
        'L1_ZDC_OR'           ,
        'L1_ZDC_XOR_E2'       ,
        'L1_ZDC_XOR_E1_E3'    ,
        'L1_ZDC_E1_AND_E1'    ,
        'L1_ZDC_E1_AND_E2ORE3',
        'L1_ZDC_E2_AND_E2'    ,
        'L1_ZDC_E2_AND_E3'    ,
        'L1_ZDC_E3_AND_E3'    ,
        'L1_ZDC_A_AND_C'      ,
        'L1_ZDC_OR_EMPTY', 'L1_ZDC_OR_UNPAIRED_NONISO',
        'L1_ZDC_A_AND_C_EMPTY', 'L1_ZDC_A_AND_C_UNPAIRED_NONISO',
        #'L1_ZDC_OR_UNPAIRED_ISO',
        #'L1_ZDC_OR_LHCF',

        # LHCF
        'L1_LHCF', 'L1_LHCF_UNPAIRED_ISO', 'L1_LHCF_EMPTY',

        # AFP
        #'L1_EM7_AFP_A_OR_C', 'L1_EM7_AFP_A_AND_C',
        'L1_MU5VF_AFP_A_OR_C', 'L1_MU5VF_AFP_A_AND_C',
        'L1_eEM9_AFP_A_OR_C','L1_eEM9_AFP_A_AND_C',

        'L1_AFP_A_OR_C_J12', 'L1_AFP_A_AND_C_J12',
        'L1_AFP_A_OR_C_jJ20', 'L1_AFP_A_AND_C_jJ20',
        'L1_AFP_A_OR_C_jJ30', 'L1_AFP_A_AND_C_jJ30',
     
        'L1_AFP_A_AND_C_TOF_J20', 'L1_AFP_A_AND_C_TOF_T0T1_J20', 
        'L1_AFP_A_AND_C_TOF_J30', 'L1_AFP_A_AND_C_TOF_T0T1_J30',
        'L1_AFP_A_AND_C_TOF_J50', 'L1_AFP_A_AND_C_TOF_T0T1_J50',
        'L1_AFP_A_AND_C_TOF_J75', 'L1_AFP_A_AND_C_TOF_T0T1_J75',

        'L1_AFP_A_AND_C_TOF_jJ50', 'L1_AFP_A_AND_C_TOF_T0T1_jJ50', 
        'L1_AFP_A_AND_C_TOF_jJ60', 'L1_AFP_A_AND_C_TOF_T0T1_jJ60',
        'L1_AFP_A_AND_C_TOF_jJ90', 'L1_AFP_A_AND_C_TOF_T0T1_jJ90', 
        'L1_AFP_A_AND_C_TOF_jJ125', 'L1_AFP_A_AND_C_TOF_T0T1_jJ125',

        'L1_AFP_A_OR_C', 'L1_AFP_A_AND_C', 'L1_AFP_A', 'L1_AFP_C', 'L1_AFP_A_AND_C_TOF_T0T1',
        'L1_AFP_FSA_BGRP12', 'L1_AFP_FSC_BGRP12', 'L1_AFP_NSA_BGRP12', 'L1_AFP_NSC_BGRP12',
        'L1_AFP_FSA_TOF_T0_BGRP12', 'L1_AFP_FSA_TOF_T1_BGRP12', 'L1_AFP_FSA_TOF_T2_BGRP12', 'L1_AFP_FSA_TOF_T3_BGRP12',
        'L1_AFP_FSC_TOF_T0_BGRP12', 'L1_AFP_FSC_TOF_T1_BGRP12', 'L1_AFP_FSC_TOF_T2_BGRP12', 'L1_AFP_FSC_TOF_T3_BGRP12',
        'L1_AFP_A_OR_C_UNPAIRED_ISO', 'L1_AFP_A_OR_C_UNPAIRED_NONISO',
        'L1_AFP_A_OR_C_EMPTY', 'L1_AFP_A_OR_C_FIRSTEMPTY',
        'L1_AFP_A_OR_C_MBTS_2', 'L1_AFP_A_AND_C_MBTS_2',
       

        # MBTS (ATR-24701)
        'L1_MBTS_1', 'L1_MBTS_1_1',  'L1_MBTS_2',
        'L1_MBTS_2_2', 'L1_MBTS_3_3',  'L1_MBTS_4_4',
        'L1_MBTS_1_EMPTY', 'L1_MBTS_1_1_EMPTY', 'L1_MBTS_2_EMPTY',
        #'L1_MBTS_1_UNPAIRED', 'L1_MBTS_2_UNPAIRED',
        'L1_MBTS_1_UNPAIRED_ISO', 'L1_MBTS_1_1_UNPAIRED_ISO', 'L1_MBTS_2_UNPAIRED_ISO',
        'L1_MBTS_2_BGRP11',
        'L1_MBTS_A', 'L1_MBTS_C',
        # extra MBTS
        #'L1_MBTSA0', 'L1_MBTSA1', 'L1_MBTSA2', 'L1_MBTSA3', 'L1_MBTSA4', 'L1_MBTSA5', 'L1_MBTSA6', 'L1_MBTSA7', 'L1_MBTSA8', 'L1_MBTSA9', 'L1_MBTSA10', 'L1_MBTSA11', 'L1_MBTSA12', 'L1_MBTSA13', 'L1_MBTSA14', 'L1_MBTSA15',
        #'L1_MBTSC0', 'L1_MBTSC1', 'L1_MBTSC2', 'L1_MBTSC3', 'L1_MBTSC4', 'L1_MBTSC5', 'L1_MBTSC6', 'L1_MBTSC7', 'L1_MBTSC8', 'L1_MBTSC9', 'L1_MBTSC10', 'L1_MBTSC11', 'L1_MBTSC12', 'L1_MBTSC13', 'L1_MBTSC14', 'L1_MBTSC15',


        # L1 items for 2022 Nov. heavy ion test run, ATR-26405
        # Additionla peripheral physics L1 items
        'L1_VTE5', 
        'L1_MBTS_1_VTE5', 
        # Additioanl supporting itesm for BeamSpot, IDCalib
        'L1_J12_VTE100',
        'L1_J30_VTE200',
        'L1_J100_VTE200', # to be checked if J100 is too high
        'L1_XE35_VTE200',
        'L1_XE50_VTE200',
 

        #--------------------------------
        # TOPO items
        #--------------------------------

        'L1_LAR-ZEE', 'L1_LAR-ZEE-eEM',

        #ATR-17320
        # 'L1_CEP-CjJ100',
        # 'L1_CEP-CjJ90' ,
    ]



# Run this file as python python/L1/Menu_MC_HI_run3_v1.py to print out available IDs
    
    L1MenuFlags.CtpIdMap = FixedIDMap

if __name__ == "__main__":
    defineMenu()
    print_available(L1MenuFlags)
