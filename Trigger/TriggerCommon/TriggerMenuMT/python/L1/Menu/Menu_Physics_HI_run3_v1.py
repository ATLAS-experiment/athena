# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Menu.MenuCommon import print_available, RequiredL1Items, defineCommonL1Flags

def defineMenu():

    defineCommonL1Flags(L1MenuFlags)

    L1MenuFlags.items = RequiredL1Items + [

        

        ##
        # single EM
        # new calo
        'L1_eEM1', 'L1_eEM2',
        'L1_eEM5', 'L1_eEM9', 'L1_eEM12', 'L1_eEM18', 'L1_eEM15',
        'L1_eEM12L', 'L1_eEM18L', 'L1_eEM26', 'L1_eEM26M',
        # ATR-22061
        "L1_eEM9_EMPTY",
        "L1_eEM15_EMPTY",
        #ATR-30312
        "L1_eEM1_EMPTY",
        "L1_eTAU1_EMPTY",
        "L1_eEM2_EMPTY",
        "L1_eEM5_EMPTY",
        # 2025 HI
        "L1_eTAU2_EMPTY",
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
        # new calo
        #'L1_2eEM7', 'L1_2eEM9', 'L1_2eEM15',
        'L1_2eEM12', 'L1_2eEM18',

        
        # combined mu - jet
        'L1_MU3V_jJ40',
        'L1_MU3V_jJ50',
        'L1_MU3V_jJ60',        

        'L1_eTAU12_EMPTY', 'L1_eTAU80', 'L1_eTAU12', 'L1_eTAU140',

        # single jet 
        # new calo
        'L1_jJ500', 'L1_jJ500_LAR',
        'L1_jJ5', 'L1_jJ10',
        'L1_jJ5p30ETA49', 'L1_jJ10p30ETA49',
        'L1_jJ20', 'L1_jJ30',
        'L1_jJ40', 'L1_jJ50', 'L1_jJ55', 'L1_jJ60', 'L1_jJ80', 'L1_jJ90',
        'L1_jJ40p30ETA49', 'L1_jJ50p30ETA49', 'L1_jJ60p30ETA49', 'L1_jJ90p30ETA49', 'L1_jJ125p30ETA49',
        'L1_jJ30_VjTE200',

        # gJ - ATR-28029
        "L1_gJ20p0ETA25","L1_gJ400p0ETA25","L1_gLJ80p0ETA25",

         # LAr saturation
        'L1_LArSaturation',

        #ATR-28679
        'L1_jXE100', 'L1_jXE110', 'L1_jXE120', 
        'L1_gXEJWOJ100', 'L1_gXEJWOJ110', 'L1_gXEJWOJ120', 'L1_gXEJWOJ500',
        'L1_jJ80_jXE120', 'L1_jJ80_jXE100',
 
        # new calo
        'L1_jTE200',
        # additional jTE items for 2023 heavy ion runs
        'L1_jTE3', 'L1_jTE4', 'L1_jTE5',
        'L1_jTE10',
        'L1_jTE20','L1_jTE50',
        'L1_jTE100',
        'L1_jTE600',
        'L1_jTE1500',
        'L1_jTE6500',
        'L1_VjTE10',
        'L1_VjTE200',
        'L1_VjTE600',
        'L1_jTE50_VjTE600',
        'L1_jTE50_VjTE200',

        # jTEFWD UCC seeds: ATR-30726
        'L1_jTEFWD2600',
        'L1_jTEFWD5600',
        'L1_jTEFWD6300',
        'L1_jTEFWD6600',
        'L1_ZDC_PU_jTEFWD5600',
        'L1_ZDC_PU_jTEFWD6300',
        'L1_ZDC_PU_jTEFWD6600',

        #Overlay items
        'L1_ZDC_A_C_VjTE50_OVERLAY', 'L1_jTE50_OVERLAY', 'L1_jTE1500_OVERLAY', 'L1_jTE4000_OVERLAY',

        #L1 forward GAP
        'L1_GAP_A', 'L1_GAP_C', 'L1_GAP_AANDC',

        #UPC - MU, phase-1 calo
        'L1_MU3V_VjTE50', 'L1_MU5VF_VjTE50', 'L1_2MU3V_VjTE50','L1_MU3V_VjTE200',
        
        #UPC - new EM
        'L1_eEM1_VjTE200', 'L1_eEM2_VjTE200', 'L1_2eEM1_VjTE200', 'L1_2eEM2_VjTE200',
        'L1_eEM5_VjTE200', 'L1_eEM9_VjTE200', 'L1_eEM9_VjTE200_EMPTY',
        'L1_eEM1_jTE4_VjTE200', 'L1_eEM2_jTE4_VjTE200', 'L1_eTAU1_jTE4_VjTE200',
        'L1_2eTAU1_VjTE200',
        'L1_eEM1_TRT_VjTE200', 'L1_eTAU1_TRT_VjTE200',
        'L1_2eTAU1_VjTE200_EMPTY','L1_2eTAU1_VjTE200_UNPAIRED_NONISO',
        'L1_eTAU1_TRT_VjTE200_EMPTY','L1_eTAU1_TRT_VjTE200_UNPAIRED_ISO','L1_eTAU1_TRT_VjTE200_UNPAIRED_NONISO',
        'L1_eEM1_TRT_VjTE50', 'L1_eTAU1_TRT_VjTE50', 'L1_DPHI-2eEM1_VjTE200_EMPTY', 'L1_DPHI-2eTAU1_VjTE200_EMPTY',

        'L1_eTAU1',
        'L1_eTAU2', 'L1_eTAU2_VjTE200', 'L1_2eTAU2_VjTE200',
        
        #UPC - TRT,  phase-1 calo
        'L1_TRT_VjTE20', 'L1_TRT_VjTE50', 'L1_TRT_VjTE200', 'L1_TRT_ZDC_XOR_VjTE200', 'L1_TRT_1ZDC_NZDC_VjTE200',
        'L1_eEM1_TRT_VjTE100', 'L1_eTAU1_TRT_VjTE100',
        'L1_eEM1_TRT_ZDC_XOR_VjTE200', 'L1_eTAU1_TRT_ZDC_XOR_VjTE200',
        'L1_eEM1_TRT_VZDC_A_VZDC_C_VjTE100', 'L1_eTAU1_TRT_VZDC_A_VZDC_C_VjTE100', 
        'L1_eEM1_TRT_ZDC_XOR4_VjTE100', 'L1_eTAU1_TRT_ZDC_XOR4_VjTE100', 
        'L1_eEM1_TRT_VjTE200_GAP_AANDC', 'L1_eTAU1_TRT_VjTE200_GAP_AANDC',
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
        #UPC magnetic monopoles
        'L1_ZDC_A_C_VjTE10','L1_ZDC_XOR_VjTE10', 'L1_ZDC_XOR_VjTE10_UNPAIRED_NONISO',
        'L1_TRT_ZDC_A_C_VjTE10','L1_TRT_ZDC_XOR_VjTE10', 'L1_TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO',
        'L1_VZDC_A_VZDC_C_VjTE50', 'L1_ZDC_A_VjTE200', 'L1_ZDC_C_VjTE200',
        'L1_TRT_ZDC_A_VjTE50', 'L1_TRT_ZDC_C_VjTE50',

        # Run3 ZDC items for heavy ion runs 
        'L1_VZDC_A_VZDC_C', #comb0
        'L1_1ZDC_A_VZDC_C', #comb4
        'L1_VZDC_A_1ZDC_C', #comb6
        'L1_1ZDC_A_1ZDC_C', #comb1
        'L1_5ZDC_A_VZDC_C', #comb5
        'L1_VZDC_A_5ZDC_C', #comb7
        'L1_ZDC_1XOR5',     #comb2
        'L1_5ZDC_A_5ZDC_C', #comb3
        
        #ZDC and phase-1 calo
        'L1_1ZDC_A_1ZDC_C_VjTE200', 'L1_ZDC_1XOR5_VjTE200',
        'L1_ZDC_XOR_VjTE200', 'L1_VZDC_A_VZDC_C_VjTE200',
        'L1_ZDC_A_C_VjTE50',

        # TRT + ZDC + Phase-1 calo
        'L1_TRT_VZDC_A_VZDC_C_VjTE200',
        'L1_TRT_ZDC_OR_VjTE200',
        'L1_TRT_ZDC_A_C_VjTE200',

        #UPC jet items
        'L1_VZDC_A_VZDC_C_jTE5_VjTE200','L1_ZDC_XOR_jTE5_VjTE200',
        'L1_1ZDC_NZDC_jTE5_VjTE200','L1_5ZDC_A_5ZDC_C_jTE5_VjTE200',
        'L1_VZDC_A_VZDC_C_jTE5_VjTE200_UNPAIRED_ISO','L1_ZDC_XOR_jTE5_VjTE200_UNPAIRED_ISO',
        'L1_VZDC_A_VZDC_C_jTE10_VjTE200', 'L1_ZDC_XOR_jTE10_VjTE200', 'L1_1ZDC_NZDC_jTE10_VjTE200',
        'L1_TRT_VZDC_A_VZDC_C_jTE5_VjTE200',
        # 'L1_ZDC_XOR_jJ5_VjTE200', 'L1_1ZDC_NZDC_jJ5_VjTE200', 'L1_VZDC_A_VZDC_C_jJ5_VjTE200',
        'L1_ZDC_XOR_jJ10_VjTE200', 'L1_1ZDC_NZDC_jJ10_VjTE200', 'L1_VZDC_A_VZDC_C_jJ10_VjTE200',
        # ATR-30727
        # 'L1_2jJ5_VjTE200', 'L1_eTAU1_jJ5_VjTE200', 'L1_jJ5_TRT_VjTE200',
        # 'L1_2jJ5_TRT_VjTE200', 'L1_eTAU1_jJ5_TRT_VjTE200',
        'L1_jJ5p30ETA49_VjTE200', 'L1_jJ10p30ETA49_VjTE200',
        # 'L1_2jJ5p30ETA49_VjTE200', 'L1_2jJ10p30ETA49_VjTE200',

        #UPC hmt trk15
        'L1_MBTS_1_VZDC_A_ZDC_C_VjTE200', 'L1_MBTS_1_1ZDC_NZDC_VjTE200',
        'L1_MBTS_1_ZDC_A_VZDC_C_VjTE200',
        'L1_MBTS_2_VZDC_A_ZDC_C_VjTE200', 'L1_MBTS_2_1ZDC_NZDC_VjTE200',
        'L1_MBTS_2_ZDC_A_VZDC_C_VjTE200',
        # ATR-30476
        'L1_MBTS_2_VZDC_A_ZDC_C_VjTE200_GAP_A', 'L1_MBTS_2_1ZDC_NZDC_VjTE200_GAP_A',
        'L1_MBTS_2_ZDC_A_VZDC_C_VjTE200_GAP_C', 'L1_MBTS_2_1ZDC_NZDC_VjTE200_GAP_C',
        #UPC hmt trk25
        'L1_VZDC_A_ZDC_C_jTE3_VjTE200', 'L1_ZDC_A_VZDC_C_jTE3_VjTE200',
        'L1_1ZDC_NZDC_jTE3_VjTE200', 
        #UPC hmt trk25 with MBTS_1
        'L1_MBTS_1_VZDC_A_ZDC_C_jTE3_VjTE200', 'L1_MBTS_1_ZDC_A_VZDC_C_jTE3_VjTE200',
        'L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200',
        'L1_MBTS_1_VZDC_A_ZDC_C_jTE3_VjTE200_GAP_A', 'L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_A',
        'L1_MBTS_1_ZDC_A_VZDC_C_jTE3_VjTE200_GAP_C', 'L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_C',
        #UPC hmt trk35
        'L1_VZDC_A_ZDC_C_jTE5_VjTE200', 'L1_ZDC_A_VZDC_C_jTE5_VjTE200',
        #UPC hmt trk35 with MBTS_1
        'L1_MBTS_1_VZDC_A_ZDC_C_jTE5_VjTE200', 'L1_MBTS_1_ZDC_A_VZDC_C_jTE5_VjTE200',
        'L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200',
        'L1_MBTS_1_VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A', 'L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200_GAP_A',
        'L1_MBTS_1_ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C', 'L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200_GAP_C',

        #UPC hmt supporting
        'L1_ZDC_OR_VjTE200_UNPAIRED_ISO', 'L1_MBTS_1_ZDC_OR_VjTE200_UNPAIRED_ISO',

        'L1_eEM1_VZDC_A_VZDC_C_VjTE100', 'L1_eEM1_ZDC_XOR4_VjTE100',
        'L1_eEM2_VZDC_A_VZDC_C_VjTE100', 'L1_eEM2_ZDC_XOR4_VjTE100',
        # ATR-30471
        'L1_TRT_ZDC_XOR_jTE5_VjTE200',

        #ZDC ucc
        'L1_ZDC_HELT20_jTEFWD2600',
        'L1_ZDC_HELT35_jTEFWD2600',
        'L1_ZDC_HELT50_jTEFWD2600',

        # VDM
        'L1_TRT_BGRP11',

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
        'L1_ZDC_A_AND_C'      ,
        'L1_ZDC_OR_EMPTY', 'L1_ZDC_OR_UNPAIRED_NONISO',
        'L1_ZDC_A_AND_C_EMPTY', 'L1_ZDC_A_AND_C_UNPAIRED_NONISO',
        #'L1_ZDC_OR_UNPAIRED_ISO',
        #'L1_ZDC_OR_LHCF',

        #ZDC pp (ATR-29027)
        # 'L1_ZDC_PP_A','L1_ZDC_PP_C','L1_ZDC_PP_OR','L1_ZDC_PP_A_C',
        # 'L1_ZDC_PP_A2','L1_ZDC_PP_C2','L1_ZDC_PP_OR2',
        # 'L1_ZDC_PP_A_EMPTY','L1_ZDC_PP_C_EMPTY',
        # 'L1_ZDC_PP_A2_EMPTY','L1_ZDC_PP_C2_EMPTY',
        # 'L1_ZDC_PP_A_UNPAIRED_NONISO','L1_ZDC_PP_C_UNPAIRED_NONISO',
        # 'L1_ZDC_PP_A2_UNPAIRED_NONISO','L1_ZDC_PP_C2_UNPAIRED_NONISO',

        # Run3 ZDC items for light ions runs (ATR-30690)
        # 'L1_ZDC_XNXN',
        # 'L1_ZDC_XNYN',
        # 'L1_ZDC_XNZN',
        # 'L1_ZDC_XN_XOR',
        # 'L1_ZDC_YN_XOR',
        # 'L1_ZDC_ZN_XOR',
        # 'L1_ZDC_YN',
        # 'L1_ZDC_ZN',
        # 'L1_ZDC_LOR',
        # 'L1_ZDC_YNYN',
        # 'L1_ZDC_LOR_EMPTY', 'L1_ZDC_LOR_UNPAIRED_NONISO',
        
        # Run3 TRT+ZDC items for light ions runs (ATR-30690)
        # 'L1_TRT_ZDC_OR',
        # 'L1_TRT_ZDC_XNXN',
        # 'L1_TRT_ZDC_XNYN',
        # 'L1_TRT_ZDC_XNZN',
        # 'L1_TRT_ZDC_XN_XOR',
        # 'L1_TRT_ZDC_YN_XOR',
        # 'L1_TRT_ZDC_ZN_XOR',
        # 'L1_TRT_ZDC_YN',
        # 'L1_TRT_ZDC_ZN',
        # 'L1_TRT_ZDC_LOR',
        # 'L1_TRT_ZDC_YNYN',
        # 'L1_TRT_ZDC_A',
        # 'L1_TRT_ZDC_C',
        # 'L1_TRT_ZDC_A_C',

        # LHCF
        'L1_LHCF', 'L1_LHCF_UNPAIRED_ISO', 'L1_LHCF_EMPTY',

        # AFP
        #'L1_EM7_AFP_A_OR_C', 'L1_EM7_AFP_A_AND_C',
        'L1_MU5VF_AFP_A_OR_C', 'L1_MU5VF_AFP_A_AND_C',
        'L1_eEM9_AFP_A_OR_C','L1_eEM9_AFP_A_AND_C',

        'L1_AFP_A_OR_C_jJ20', 'L1_AFP_A_AND_C_jJ20',
        'L1_AFP_A_OR_C_jJ30', 'L1_AFP_A_AND_C_jJ30',

        'L1_AFP_A_AND_C_TOF_jJ50', 'L1_AFP_A_AND_C_TOF_T0T1_jJ50', 
        'L1_AFP_A_AND_C_TOF_jJ60', 'L1_AFP_A_AND_C_TOF_T0T1_jJ60',
        'L1_AFP_A_AND_C_TOF_jJ90', 'L1_AFP_A_AND_C_TOF_T0T1_jJ90', 
        'L1_AFP_A_AND_C_TOF_jJ125', 'L1_AFP_A_AND_C_TOF_T0T1_jJ125',

        'L1_AFP_A_OR_C', 'L1_AFP_A_AND_C', 'L1_AFP_A', 'L1_AFP_C', 'L1_AFP_A_AND_C_TOF_T0T1', 'L1_AFP_A_AND_C_TOF', 
        'L1_AFP_FSA_BGRP12', 'L1_AFP_FSC_BGRP12', 'L1_AFP_NSA_BGRP12', 'L1_AFP_NSC_BGRP12',
        'L1_AFP_FSA_TOF_T0_BGRP12', 'L1_AFP_FSA_TOF_T1_BGRP12', 'L1_AFP_FSA_TOF_T2_BGRP12', 'L1_AFP_FSA_TOF_T3_BGRP12',
        'L1_AFP_FSC_TOF_T0_BGRP12', 'L1_AFP_FSC_TOF_T1_BGRP12', 'L1_AFP_FSC_TOF_T2_BGRP12', 'L1_AFP_FSC_TOF_T3_BGRP12',
        'L1_AFP_A_OR_C_UNPAIRED_ISO', 'L1_AFP_A_OR_C_UNPAIRED_NONISO',
        'L1_AFP_A_OR_C_EMPTY', 'L1_AFP_A_OR_C_FIRSTEMPTY',
        'L1_AFP_A_OR_C_TOF_UNPAIRED_ISO', 'L1_AFP_A_OR_C_TOF_UNPAIRED_NONISO',
        'L1_AFP_A_OR_C_TOF_EMPTY', 'L1_AFP_A_OR_C_TOF_FIRSTEMPTY',
       

        # MBTS (ATR-24701)
        'L1_MBTS_1', 'L1_MBTS_1_1',  'L1_MBTS_2',
        'L1_MBTS_2_2', 'L1_MBTS_3_3',  'L1_MBTS_4_4',
        'L1_MBTS_1_EMPTY', 'L1_MBTS_1_1_EMPTY', 'L1_MBTS_2_EMPTY',
        #'L1_MBTS_1_UNPAIRED', 'L1_MBTS_2_UNPAIRED',
        'L1_MBTS_1_UNPAIRED_ISO', 'L1_MBTS_1_1_UNPAIRED_ISO', 'L1_MBTS_2_UNPAIRED_ISO',
        'L1_MBTS_2_BGRP11',
        'L1_MBTS_A', 'L1_MBTS_C',
        'L1_MBTS_1_1_VjTE50',
        # extra MBTS
        'L1_MBTSA0', 'L1_MBTSA1', 'L1_MBTSA2', 'L1_MBTSA3', 'L1_MBTSA4', 'L1_MBTSA5', 'L1_MBTSA6', 'L1_MBTSA7', 'L1_MBTSA8', 'L1_MBTSA9', 'L1_MBTSA10', 'L1_MBTSA11', 'L1_MBTSA12', 'L1_MBTSA13', 'L1_MBTSA14', 'L1_MBTSA15',
        'L1_MBTSC0', 'L1_MBTSC1', 'L1_MBTSC2', 'L1_MBTSC3', 'L1_MBTSC4', 'L1_MBTSC5', 'L1_MBTSC6', 'L1_MBTSC7', 'L1_MBTSC8', 'L1_MBTSC9', 'L1_MBTSC10', 'L1_MBTSC11', 'L1_MBTSC12', 'L1_MBTSC13', 'L1_MBTSC14', 'L1_MBTSC15',



        #--------------------------------
        # TOPO items
        #--------------------------------

        'L1_LAR-ZEE-eEM',
        #ATR-30145
        'L1_JPSI-1M5-eEM9',
        #ATR-29784
        'L1_DPHI-2eEM1','L1_DPHI-2eTAU1',
        'L1_DPHI-2eEM1_VjTE200','L1_DPHI-2eTAU1_VjTE200',
        # 'L1_DPHI-2eEM1_VjTE200_GAP_AANDC','L1_DPHI-2eTAU1_VjTE200_GAP_AANDC',

        # ATR-30728
        'L1_23INVM-24DPHI-2eTAU1_VjTE200',
        'L1_28INVM-24DPHI-2eTAU1_VjTE200',
        'L1_23INVM-25DPHI-2eTAU1_VjTE200',
        'L1_33INVM-25DPHI-2eTAU1_VjTE200',
        'L1_23INVM-27DPHI-2eTAU1_VjTE200',

        'L1_23INVM-27DPHI-2eTAU1_VjTE200_EMPTY',
        'L1_23INVM-27DPHI-2eTAU1_VjTE200_UNPAIRED_ISO',
        'L1_23INVM-27DPHI-2eTAU1_VjTE200_UNPAIRED_NONISO',

         # ATR-31097
        'L1_TeAsymmetry-jTENoSort',
        'L1_TeAsymmetry1-jTENoSort',
        'L1_TeAsymmetry2-jTENoSort',
        'L1_TeAsymmetry3-jTENoSort',
        'L1_TeATIME-jTENoSort',
        'L1_ESPRESSO',

        #ATR-28678 Ph1 Items for Phisics_pp_Run3
        "L1_jJ30_BGRP12",
        "L1_jJ30_EMPTY",
        "L1_jJ30_FIRSTEMPTY",
        "L1_jJ30_UNPAIRED_ISO",
        "L1_jJ30_UNPAIRED_NONISO",
        "L1_jJ30_UNPAIREDB1",
        "L1_jJ30_UNPAIREDB2",

        "L1_jJ60_EMPTY",
        "L1_jJ60_FIRSTEMPTY",
        "L1_jJ60p30ETA49_EMPTY",

        "L1_jJ90_UNPAIRED_ISO",
        "L1_jJ90_UNPAIRED_NONISO",

        "L1_jJ125",

        "L1_jJ160",

        "L1_TEA_TeAsymmetry-jTENoSort",
        "L1_TEA_eEM2",
        "L1_TEA_eTAU2",
        "L1_TEA_jJ5",
        "L1_TEA_jJ5p30ETA49",
        "L1_ESP_TeAsymmetry-jTENoSort",
        "L1_ESP_eEM2",
        "L1_ESP_eTAU2",
        "L1_ESP_jJ5",
        "L1_ESP_jJ5p30ETA49",

        # jJ + ZDC + TeATIME for 2025 HI
        'L1_TEA_1ZDC_NZDC_jJ10_VjTE200',
        'L1_TEA_1ZDC_NZDC_jJ5_VjTE200',
        'L1_TEA_5ZDC_A_5ZDC_C_jJ10_VjTE200',
        'L1_TEA_5ZDC_A_5ZDC_C_jJ5_VjTE200',
        'L1_TEA_VZDC_A_VZDC_C_jJ10_VjTE200',
        'L1_TEA_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200',
        'L1_TEA_VZDC_A_VZDC_C_jJ5_VjTE200',
        'L1_TEA_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',
        'L1_TEA_ZDC_5XOR_jJ10_VjTE200',
        'L1_TEA_ZDC_5XOR_jJ5_VjTE200',
        'L1_TEA_ZDC_XOR_jJ10_VjTE200',
        'L1_TEA_ZDC_XOR_jJ10p30ETA49_VjTE200',
        'L1_TEA_ZDC_XOR_jJ5_VjTE200',
        'L1_TEA_ZDC_XOR_jJ5p30ETA49_VjTE200',

        # UPC HMT with TeAsymmetry for 2025 HI
        'L1_TEA_ASYM0_TRT_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM1_TRT_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM2_TRT_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM3_TRT_ZDC_XOR_VjTE200',

        'L1_TEA_ASYM0_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM1_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM2_ZDC_XOR_VjTE200',
        'L1_TEA_ASYM3_ZDC_XOR_VjTE200',

        # Ditaus for 2025 HI
        'L1_eEM2_TRT_VZDC_A_VZDC_C_VjTE200',
        'L1_eEM2_TRT_ZDC_OR_VjTE200',
        'L1_eTAU2_TRT_VZDC_A_VZDC_C_VjTE200',
        'L1_eTAU2_TRT_ZDC_OR_VjTE200',

        # for LAr (2025 HI)
        'L1_jJ5_EMPTY',
        'L1_jJ10_EMPTY',

        # supporting for UPC egamma
        'L1_eEM2_VjTE200_EMPTY',
        'L1_eEM5_VjTE200_EMPTY',
        'L1_eTAU2_VjTE200_EMPTY',

        'L1_CALMTEA_eEM2',
        'L1_CALMTEA_eTAU2',

        'L1_CALMTEA_eEM2_VjTE200',
        'L1_CALMTEA_eTAU2_VjTE200',

        'L1_ESP_1ZDC_NZDC_jJ10_VjTE200',
        'L1_ESP_5ZDC_A_5ZDC_C_jJ10_VjTE200',
        'L1_ESP_VZDC_A_VZDC_C_jJ10_VjTE200',
        'L1_ESP_VZDC_A_VZDC_C_jJ10p30ETA49_VjTE200',
        'L1_ESP_VZDC_A_VZDC_C_jJ5p30ETA49_VjTE200',
        'L1_ESP_ZDC_5XOR_jJ10_VjTE200',
        'L1_ESP_ZDC_XOR_jJ10_VjTE200',
        'L1_ESP_ZDC_XOR_jJ10p30ETA49_VjTE200',
        'L1_ESP_ZDC_XOR_jJ5p30ETA49_VjTE200',

        'L1_ESP_ASYM0_TRT_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM1_TRT_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM2_TRT_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM3_TRT_ZDC_XOR_VjTE200',

        'L1_ESP_ASYM0_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM1_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM2_ZDC_XOR_VjTE200',
        'L1_ESP_ASYM3_ZDC_XOR_VjTE200',

        'L1_MATCHA_eEM2',
        'L1_MATCHA_eTAU2',

        'L1_MATCHA_eEM2_VjTE200',
        'L1_MATCHA_eTAU2_VjTE200',

        'L1_MATCHA_eEM5_VjTE200',

        'L1_MATCHA_eEM2_EMPTY',
        'L1_MATCHA_eTAU2_EMPTY',

        'L1_MATCHA_eEM2_VjTE200_EMPTY',
        'L1_MATCHA_eTAU2_VjTE200_EMPTY',

        'L1_MATCHA_eEM5_VjTE200_EMPTY',
    ]


if __name__ == "__main__":
    defineMenu()
    print_available(L1MenuFlags)
