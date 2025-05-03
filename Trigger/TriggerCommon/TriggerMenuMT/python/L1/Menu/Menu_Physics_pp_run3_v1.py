# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Menu.MenuCommon import print_available, RequiredL1Items, defineCommonL1Flags

def defineMenu():

    defineCommonL1Flags(L1MenuFlags)

    L1MenuFlags.items = RequiredL1Items + [

        ##
        # single EM
        ##
        # new calo
        'L1_eEM5', 'L1_eEM7', 'L1_eEM9', 'L1_eEM10L',
        'L1_eEM12L', 'L1_eEM15', 'L1_eEM18', 'L1_eEM18L', 'L1_eEM18M',
        'L1_eEM22M', 'L1_eEM24L', 'L1_eEM24VM',
        'L1_eEM5_EMPTY','L1_eEM9_EMPTY', 'L1_eEM9_FIRSTEMPTY','L1_2eEM9_EMPTY', 'L1_eEM9_UNPAIRED_ISO',
        'L1_eEM15_EMPTY',
        'L1_eEM26', 'L1_eEM26L', 'L1_eEM26M', 'L1_eEM26T', 'L1_eEM28M', 'L1_eEM40L_2eEM18L',
        #beam splashes
        'L1_eEM22A', 'L1_eEM22C',
        
        ## 
        # MU
        ##
        'L1_MU3V', 'L1_MU5VF', 'L1_MU8F', 'L1_MU8VF', 'L1_MU14FCH', 'L1_MU14FCHR',
        'L1_MU3VF', 'L1_MU8FC', 'L1_MU8VFC', 'L1_MU15VFCH', 'L1_MU10BOM', 'L1_MU4BOM', 'L1_MU12BOM', 'L1_MU10BO', 
        'L1_2MU3V', 'L1_2MU3VF', 'L1_2MU5VF', 'L1_2MU8F', 'L1_MU8VF_2MU5VF', 'L1_MU5VF_2MU3V', 'L1_MU5VF_2MU3VF',
        'L1_3MU3V', 'L1_3MU3VF', 'L1_3MU5VF', 'L1_MU5VF_3MU3V', 'L1_MU5VF_3MU3VF', 'L1_4MU3V', 
        'L1_2MU5VF_3MU3V', 'L1_2MU8VF', 
        'L1_MU8F_2MU5VF',

        'L1_2MU14FCH_OVERLAY',
        'L1_MU3V_EMPTY', 'L1_2MU5VF_EMPTY', 'L1_MU3V_FIRSTEMPTY', 'L1_MU8VF_EMPTY',
        'L1_MU3V_UNPAIRED_ISO',        

        # test
        'L1_MU3VC', 'L1_MU4BO', 'L1_MU3EOF', 
        'L1_MU8FH', 'L1_MU8EOF', 'L1_MU9VF', 'L1_MU9VFC', 
        'L1_MU12FCH', 'L1_MU14EOF', 'L1_MU15VFCHR', 'L1_MU18VFCH', 'L1_MU20VFC', 

        ##
        # Combined lepton, new calo (for ATR-24182)
        'L1_2eEM10L',  # prospective Run 4 L1 item, ATR-30180
        'L1_2eEM18', 'L1_2eEM18L', 'L1_2eEM18M', 'L1_2eEM24L', 'L1_3eEM12L', 'L1_eEM24L_3eEM12L',
        'L1_eEM18L_MU8F', 'L1_2eEM10L_MU8F',
        # ATR-27156
        'L1_2eEM9',
        # single tau
        'L1_eTAU12', 'L1_cTAU12M', 'L1_eTAU12_EMPTY', 'L1_eTAU12_FIRSTEMPTY', 'L1_eTAU12_UNPAIRED_ISO',
        'L1_eTAU20', 'L1_jTAU20', 'L1_cTAU20M',
        'L1_eTAU20L', 'L1_eTAU20M', 
        'L1_eTAU30', 'L1_cTAU30M', 
        'L1_eTAU35', 'L1_cTAU35M', 
        'L1_eTAU40HM', 'L1_eTAU40HT', 
        'L1_cTAU50M',
        'L1_eTAU60', 'L1_eTAU60HM', 'L1_eTAU60_EMPTY', 'L1_eTAU60_UNPAIRED_ISO',
        'L1_eTAU70', 
        'L1_eTAU80', 'L1_eTAU140',



        # multi tau, new calo
        'L1_eTAU80_2eTAU60', 
        'L1_cTAU30M_2cTAU20M_4jJ30p0ETA25',
        'L1_cTAU35M_2cTAU30M_2jJ55_3jJ50',  
        'L1_cTAU35M_2cTAU30M',
        'L1_cTAU30M_2cTAU20M_jJ55_2jJ50_3jJ30',
        
        # combined tau - lepton
        #Phase-I
        'L1_MU8F_cTAU20M',
        'L1_MU8F_cTAU20M_jJ55_2jJ30',
        'L1_MU8F_cTAU20M_3jJ30',
        'L1_eEM18M_2cTAU20M_4jJ30',
        
        # combined tau - xe
        'L1_eEM18M_2cTAU20M_jXE70',
        'L1_MU8F_cTAU20M_jXE70',
        'L1_eTAU60_2cTAU20M_jXE80',


        # ATR-28761 Phase1 combined em - jet
        'L1_eEM22M_3jJ50',
        'L1_eEM24L_3jJ50',

        # combined mu - jet 
        'L1_BTAG-MU3VjJ40', 'L1_BTAG-MU5VFjJ80',

        'L1_BTAG-MU3VFjJ40_2jJ30p0ETA25',
        'L1_BTAG-MU3VFjJ40_2jJ30p0ETA25_jJ50p0ETA25',
        'L1_BTAG-MU3VFjJ40_2jJ40p0ETA25',
        'L1_BTAG-MU3VFjJ40_2jJ40p0ETA25_jJ50p0ETA25',

        'L1_BTAG-MU5VFjJ40_2jJ30p0ETA25',
        'L1_BTAG-MU5VFjJ40_2jJ30p0ETA25_jJ50p0ETA25',
        'L1_BTAG-MU5VFjJ40_2jJ40p0ETA25',
        'L1_BTAG-MU5VFjJ40_2jJ40p0ETA25_jJ50p0ETA25',

        #ATR-13743 J,XE thershold change for ATR-19376 
        'L1_MU8F_2jJ50','L1_MU8F_3jJ50', 'L1_MU8F_2jJ40_jJ50',
        'L1_MU14FCH_jJ90',
        'L1_MU14FCH_jXE80',
        'L1_MU14FCH_EMPTY',
        'L1_MU14FCH_UNPAIRED_ISO',

        # jJ
        'L1_jJ30', 'L1_jJ30_BGRP12','L1_jJ30_EMPTY','L1_jJ30_FIRSTEMPTY',
        'L1_jJ30_UNPAIRED_ISO','L1_jJ30_UNPAIRED_NONISO','L1_jJ30_UNPAIREDB1','L1_jJ30_UNPAIREDB2',
        'L1_jJ30p0ETA25',
         
        'L1_jJ40', 'L1_jJ40p0ETA25', 'L1_jJ40p30ETA49', 'L1_jJ40p30ETA49_UNPAIRED_ISO',

        'L1_jJ50', 'L1_jJ50p30ETA49',

        'L1_jJ55', 'L1_jJ55p0ETA23', 'L1_jJ55p0ETA23_2jJ40p30ETA49', 

        'L1_jJ60', 'L1_jJ60_EMPTY', 'L1_jJ60_FIRSTEMPTY','L1_jJ60p30ETA49',  
        'L1_jJ60p30ETA49_EMPTY', 'L1_jJ60p30ETA49_UNPAIRED_ISO', 'L1_jJ60p30ETA49_UNPAIRED_NONISO',

        'L1_jJ70p0ETA23', 

        'L1_jJ80', 
        'L1_jJ80p0ETA25', 'L1_jJ80p0ETA25_2jJ40p30ETA49', 'L1_jJ80p0ETA25_2jJ55_jJ50p30ETA49', 

        'L1_jJ85p0ETA21', 'L1_jJ85p0ETA21_3jJ40p0ETA25', 

        'L1_jJ90', 'L1_jJ90_UNPAIRED_ISO', 'L1_jJ90_UNPAIRED_NONISO',
        'L1_jJ90p30ETA49', 
        'L1_jJ90_2jJ80p0ETA25_3jJ40p0ETA25',

        'L1_jJ125', 'L1_jJ125p30ETA49',

        'L1_jJ140',  

        'L1_jJ160', 'L1_jJ160_FIRSTEMPTY',

        'L1_jJ180', 

        'L1_jJ500', 'L1_jJ500_LAR',

        'L1_3jJ55p0ETA23',
        'L1_6jJ40',

        'L1_4jJ40', 'L1_3jJ90', 'L1_4jJ50', 'L1_4jJ40p0ETA25', 'L1_5jJ40p0ETA25', 
        'L1_3jJ70p0ETA23', 'L1_jJ140_3jJ60', 
        'L1_MU3V_jJ30', 'L1_MU3V_jJ40', 'L1_MU5VF_jJ80',  
        #Kept as Phase-1 ATR-28761 
  
        # jLJ
        'L1_jLJ80', 'L1_jLJ120', 'L1_jLJ140', 'L1_jLJ180',

        # jEM
        'L1_jEM25', 'L1_jEM20M',
        'L1_jEM35', # prospective Run 4 L1 item, ATR-30180

        # gJ
        'L1_gJ20p0ETA25', 'L1_gJ20p0ETA25_EMPTY', 'L1_gJ20p25ETA49', 'L1_gJ50p0ETA25', 'L1_gJ100p0ETA25', 'L1_gJ400p0ETA25',

        # gLJ
        'L1_gLJ80p0ETA25', 'L1_gLJ100p0ETA25', 'L1_gLJ140p0ETA25', 'L1_gLJ160p0ETA25',
        'L1_gLJ90p0ETA25', # prospective Run 4 L1 item, ATR-30180

        # LAr saturation
        'L1_LArSaturation',

        # combined jet
        'L1_jJ80_jXE100',
        # ATR-27250 Duplicate multijet-seeded triggers to jFEX
        'L1_2jJ90_jXE80', 
        'L1_2jJ40_jXE110',
        'L1_3jJ40p0ETA25_jXE80',

        'L1_2jJ56p0ETA49', # prospective Run 4 L1 item, ATR-30180
        #ATR-29523
        'L1_3jJ40p0ETA25',
        # new calo
        'L1_gXENC70', 'L1_gXENC100',
        'L1_gXEJWOJ60', 'L1_gXEJWOJ70', 'L1_gXEJWOJ80', 'L1_gXEJWOJ100', 'L1_gXEJWOJ110', 'L1_gXEJWOJ120', 'L1_gXEJWOJ500',
        'L1_gTE200',
        'L1_gMHT500',

        'L1_jXE60', 'L1_jXE70', 'L1_jXE80', 'L1_jXE90', 'L1_jXE100', 'L1_jXE110', 'L1_jXE120', 'L1_jXE500', 
        'L1_jXEC100', 'L1_jTE200', 'L1_jTEC200', 'L1_jTEFWD100', 'L1_jTEFWDA100', 'L1_jTEFWDC100',
    
        #LUCID
        'L1_LUCID_A', 'L1_LUCID_C',
        'L1_LUCID_A_BGRP11', 'L1_LUCID_C_BGRP11',

        #combined jet xe
        'L1_jJ80_jXE120',

        # VDM

        # LHCF

        # AFP
        # high-priority (all mu)
        'L1_AFP_A_AND_C_TOF_T0T1',
        'L1_AFP_FSA_BGRP12', 'L1_AFP_FSC_BGRP12',
        # med-priority (all mu)
        'L1_AFP_FSA_TOF_T0_BGRP12', 'L1_AFP_FSA_TOF_T1_BGRP12','L1_AFP_FSC_TOF_T0_BGRP12', 'L1_AFP_FSC_TOF_T1_BGRP12',
        # low-priority (all mu)
        'L1_AFP_FSA_TOF_T2_BGRP12', 'L1_AFP_FSA_TOF_T3_BGRP12','L1_AFP_FSC_TOF_T2_BGRP12', 'L1_AFP_FSC_TOF_T3_BGRP12',
        'L1_AFP_A_OR_C_UNPAIRED_ISO', 'L1_AFP_A_OR_C_UNPAIRED_NONISO', 'L1_AFP_A_OR_C_EMPTY', 'L1_AFP_A_OR_C_FIRSTEMPTY',
        
        'L1_AFP_A_OR_C_TOF_UNPAIRED_ISO', 'L1_AFP_A_OR_C_TOF_UNPAIRED_NONISO', 'L1_AFP_A_OR_C_TOF_EMPTY', 'L1_AFP_A_OR_C_TOF_FIRSTEMPTY',
        'L1_AFP_A_AND_C_TOF_jJ50', 'L1_AFP_A_AND_C_TOF_T0T1_jJ50', 'L1_AFP_A_AND_C_TOF_jJ60', 'L1_AFP_A_AND_C_TOF_T0T1_jJ60', 'L1_AFP_A_AND_C_TOF_jJ90', 'L1_AFP_A_AND_C_TOF_T0T1_jJ90', 'L1_AFP_A_AND_C_TOF_jJ125', 'L1_AFP_A_AND_C_TOF_T0T1_jJ125',

        # high-priority (low mu)
        'L1_AFP_NSA_BGRP12', 'L1_AFP_NSC_BGRP12', 
        'L1_AFP_A','L1_AFP_C', 'L1_AFP_A_AND_C', 'L1_AFP_A_AND_C_TOF',
        'L1_AFP_A_OR_C_jJ30', 'L1_AFP_A_AND_C_jJ30',
        'L1_MU5VF_AFP_A_OR_C', 'L1_MU5VF_AFP_A_AND_C',
        'L1_eEM9_AFP_A_OR_C', 'L1_eEM9_AFP_A_AND_C',
        # med-priority (low mu)
        'L1_AFP_A_OR_C', 'L1_AFP_A_OR_C_MBTS_2', 'L1_AFP_A_AND_C_MBTS_2',
        

        # MBTS
        'L1_MBTS_A', 'L1_MBTS_C',
        'L1_MBTS_1_EMPTY', 'L1_MBTS_1_1_EMPTY', 'L1_MBTS_2_EMPTY', 
        'L1_MBTS_1_UNPAIRED_ISO', 'L1_MBTS_1_1_UNPAIRED_ISO', 'L1_MBTS_2_UNPAIRED_ISO',
        'L1_MBTS_1', 'L1_MBTS_1_1', 'L1_MBTS_2',
        'L1_MBTS_4_A', 'L1_MBTS_4_C',
        'L1_MBTS_1_A', 'L1_MBTS_1_C',
        'L1_MBTS_1_A_EMPTY', 'L1_MBTS_1_C_EMPTY',
        # For VdM
        'L1_MBTS_2_BGRP11', 'L1_MBTS_1_1_BGRP11', 'L1_TRT_BGRP11',

        # extra MBTS 
        # TODO: to be removed for high-mu pp        
        'L1_MBTSA0', 'L1_MBTSA1', 'L1_MBTSA2', 'L1_MBTSA3', 'L1_MBTSA4', 'L1_MBTSA5', 'L1_MBTSA6', 'L1_MBTSA7', 'L1_MBTSA8', 'L1_MBTSA9', 'L1_MBTSA10', 'L1_MBTSA11', 'L1_MBTSA12', 'L1_MBTSA13', 'L1_MBTSA14', 'L1_MBTSA15', 'L1_MBTSC0', 'L1_MBTSC1', 'L1_MBTSC2', 'L1_MBTSC3', 'L1_MBTSC4', 'L1_MBTSC5', 'L1_MBTSC6', 'L1_MBTSC7', 'L1_MBTSC8', 'L1_MBTSC9', 'L1_MBTSC10', 'L1_MBTSC11', 'L1_MBTSC12', 'L1_MBTSC13', 'L1_MBTSC14', 'L1_MBTSC15', 



        #--------------------------------
        # TOPO items
        #--------------------------------

        # phase1 L1Topo
        'L1_LAR-ZEE-eEM',
        'L1_JPSI-1M5-eEM9',
        'L1_JPSI-1M5-eEM15',
        'L1_BPH-0M9-eEM9-eEM7', 'L1_BPH-0M9-eEM9-eEM7_MU5VF', 'L1_BPH-0M9-eEM9-eEM7_2MU3V',
        'L1_BPH-0DR3-eEM9jJ40', 'L1_BPH-0DR3-eEM9jJ40_MU5VF', 'L1_BPH-0DR3-eEM9jJ40_2MU3V',
        'L1_LLP-RO-eEM', 'L1_LLP-NOMATCH-eEM',
        #'L1_DPHI-2eEM5', 
        'L1_HT150-jJ50s5pETA32_jMJJ-400-CF',
        'L1_HT190-jJ40s5pETA21',
        'L1_SC175-SCjJ10',  
        'L1_jJ90_DETA20-jJ90J', 
        #ATR-30618
        'L1_ADVAET',
        'L1_ADVAEL',

        # tau 
        'L1_cTAU30M_2cTAU20M',
        'L1_cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L', 'L1_cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L-jJ55',
        'L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20', 'L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55', 
        'L1_eTAU80_2cTAU30M_DR-eTAU30eTAU20', 
        'L1_cTAU20M_DR-eTAU20eTAU12-jJ40', 
        # ATR-26902
        'L1_4jJ30p0ETA24_0DETA24-eTAU30eTAU12',
        'L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24-eTAU30eTAU12',
        'L1_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12',
        # ATR-30638 (g-2 tau measurement)
        'L1_eTAU70_2cTAU50M_DPHI-2eTAU50',
        'L1_2cTAU50M_DPHI-2eTAU50',
        
        'L1_DY-BOX-2MU5VF', 'L1_DY-BOX-MU5VFMU3V', 'L1_DY-BOX-2MU3VF',

        #ATR-23394
        'L1_LFV-eEM10L-MU8VF', 'L1_LFV-eEM15L-MU5VF',

        'L1_LFV-MU5VF',
        'L1_LFV-MU8VF', 

        'L1_jMJJ-700',
        'L1_jMJJ-300-NFF',
        'L1_jMJJ-500-NFF',
        'L1_jMJJ-400-CF',
        'L1_eEM22M_jMJJ-300',

        'L1_10DR-MU14FCH-MU5VF', #ATR-19376
        'L1_10DR-MU14FCH-MU5VF_EMPTY',
        'L1_10DR-MU14FCH-MU5VF_UNPAIRED_ISO',
    
        #ATR-19720, ATR-19639
        'L1_BPH-2M9-0DR15-2MU3V',
        'L1_BPH-2M9-0DR15-2MU3VF',
        'L1_BPH-2M9-0DR15-MU5VFMU3V',
        'L1_BPH-2M9-0DR15-C-MU5VFMU3V',
        'L1_BPH-2M9-2DR15-2MU5VF',
        'L1_BPH-8M15-0DR22-MU5VFMU3V-BO',
        'L1_BPH-8M15-0DR22-2MU5VF',
        #ATR-19355
        'L1_BPH-0M10-3MU3V',
        'L1_BPH-0M10-3MU3VF',
        #ATR-19638
        'L1_BPH-0M10C-3MU3V',

        #ATR-21566 
        'L1_BPH-7M22-2MU3VF',
        'L1_BPH-7M22-MU5VFMU3VF',
        'L1_BPH-7M22-0DR20-2MU3V',
        'L1_BPH-7M22-0DR20-2MU3VF', 
        'L1_BPH-7M22-0DR12-2MU3V',

        #ATR-22782
        'L1_BPH-7M11-25DR99-2MU3VF',
        'L1_BPH-7M14-MU5VFMU3VF',
        'L1_BPH-7M14-0DR25-MU5VFMU3VF',
        'L1_BPH-7M14-2MU3V', 
        'L1_BPH-7M14-2MU3VF',

        # INVM + DPHI 
        'L1_jMJJ-400-NFF-0DPHI22',
        'L1_jMJJ-400-NFF-0DPHI24',
        'L1_jMJJ-400-NFF-0DPHI26',

        'L1_LATE-MU8F_jXE70', 'L1_LATE-MU8F_jJ90',

        # INVM + DR, TLA 
        'L1_2DR15-0M30-2eEM12L',
        'L1_13DR25-25M70-2eEM12L',

        #ATR-18824
        'L1_ZAFB-04DPHI-eEM18M',
        'L1_ZAFB-25DPHI-eEM18M',
        #ATR-30666
        'L1_ZAFB-04DPHIM-eEM18M',
        'L1_ZAFB-25DPHIM-eEM18M',
        #ATR-30822
        'L1_eEM18M_jEM25',

        #ATR-22109
        #'L1_ZAFB-25DPHI-eEM18M',

        'L1_DPHI-M70-2eEM12M', 'L1_DPHI-M70-2eEM15M', #ATR-19302
        'L1_DPHI-M70-2eEM9', 'L1_DPHI-M70-2eEM9L', # ATR-21637 (no or loose shower shape cuts)
                
        #ATR-17320
        'L1_CEP-CjJ100',
        'L1_CEP-CjJ90',
        'L1_AFP_A_AND_C_TOF_CEP-CjJ100','L1_AFP_A_AND_C_TOF_T0T1_CEP-CjJ100',

        #ATR-28563
        'L1_LLPDPHI-jXE40-jJ40',
        'L1_LLPNODPHI-jXE40-jJ40',

        #ATR-30656
        'L1_cTAU30M_3DR35-MU8F-eTAU30',
        
        ]


if __name__ == "__main__":
    defineMenu()
    print_available(L1MenuFlags)


