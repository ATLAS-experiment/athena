# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_Physics_HI_run3_v1 as physics_menu

def defineMenu():
    physics_menu.defineMenu()

    # Add new items to the item list in the Physics menu
    l1items = L1MenuFlags.items()
    l1items += [

        # legacy EM
        #'L1_EM8VH',
        'L1_EM10', 'L1_EM10VH', 'L1_EM12',
        #'L1_EM14', 
        'L1_EM15', 
        #'L1_EM16','L1_EM18VH', 
        #'L1_EM20VHI', 'L1_EM22',
        #'L1_EM22VHI',
        #'L1_EM20VH_FIRSTEMPTY',
        'L1_2EM15',
        #'L1_2EM10', 'L1_2EM16',
        #'L1_2EM20VH',

        #combined mu+jet
        'L1_MU3V_J12',
        'L1_MU3V_J20',
        'L1_MU3V_J30',        

        #single jet
        'L1_J12', 'L1_J15','L1_J20', 'L1_J25', 'L1_J30', 'L1_J40', 
        'L1_J50' ,'L1_J75','L1_J85', 'L1_J100', 'L1_J400',
        'L1_J15p31ETA49', 'L1_J20p31ETA49', 'L1_J30p31ETA49', 'L1_J50p31ETA49', 'L1_J75p31ETA49', 
        'L1_J400_LAR',
        
        # XE
        'L1_XE50', 'L1_XE55', 
        'L1_XE300',
       
        'L1_J40_XE50', 'L1_J40_XE60',

         # calo
        'L1_TE3', 
        #'L1_TE4', 
        'L1_TE5', # also for HMT triggers
        'L1_TE10', 'L1_TE20', 'L1_TE50', 'L1_TE100', 
        'L1_TE200',
        'L1_TE3p0ETA49', 'L1_TE7p0ETA49',
        'L1_TE600p0ETA49', 'L1_TE1500p0ETA49', 'L1_TE3000p0ETA49', 'L1_TE3500p0ETA49', 'L1_TE6500p0ETA49', 'L1_TE8000p0ETA49',
        'L1_TE50_VTE600p0ETA49',

        #UPC - MU
        'L1_MU3V_VTE50', 'L1_MU5VF_VTE50', 'L1_2MU3V_VTE50', 'L1_MU3V_VTE200',
        
        #UPC - EM
        'L1_TAU1_TE4_VTE200', 'L1_TAU1_TE5_VTE200',
        'L1_TAU2_TE4_VTE200',
        'L1_2TAU1_VTE200',
        'L1_TAU8_VTE200',
        #'L1_EM7_VTE200',
        
        'L1_eEM1_TE4_VTE200', 'L1_eEM2_TE4_VTE200',
        'L1_eEM1_VTE200', 'L1_2eEM1_VTE200', 'L1_2eEM2_VTE200', 'L1_eEM9_VTE200',

        #UPC - calo
        'L1_ZDC_XOR_VTE200', 'L1_VZDC_A_VZDC_C_TE5_VTE200',
        'L1_VZDC_A_VZDC_C_TE10_VTE200',
        'L1_ZDC_A_VZDC_C_VTE200', 'L1_VZDC_A_ZDC_C_VTE200',
        
        'L1_MBTS_1_VTE50',
        #UPC - calo, TRT - legacy
        'L1_TRT_VTE50',
        'L1_TRT_VTE200',
        'L1_TRT_VTE20',
        'L1_TAU1_TRT_VTE50',

        #UPC - calo only - legacy
        'L1_VTE20',
        'L1_VTE50', 'L1_TE3_VTE50',
        'L1_VTE200', 'L1_TE5_VTE200', 'L1_TE50_VTE200',
        #'L1_J12_VTE200',

        #ZDC
        'L1_ZDC_A_VTE200', 'L1_ZDC_C_VTE200',
        'L1_TRT_ZDC_A_VTE50', 'L1_TRT_ZDC_C_VTE50',
        #ZDC and legacy calo
        'L1_1ZDC_A_1ZDC_C_VTE200', 'L1_ZDC_1XOR5_VTE200',
        'L1_VZDC_A_ZDC_C_TE3_VTE200', 'L1_1ZDC_A_1ZDC_C_TE3_VTE200',
        'L1_ZDC_1XOR5_TE3_VTE200', 'L1_ZDC_A_VZDC_C_TE3_VTE200',
        'L1_VZDC_A_ZDC_C_TE5_VTE200', 'L1_1ZDC_A_1ZDC_C_TE5_VTE200',
        'L1_ZDC_1XOR5_TE5_VTE200', 'L1_ZDC_A_VZDC_C_TE5_VTE200',
        'L1_ZDC_XOR_TE3_VTE200', 'L1_ZDC_XOR_TE5_VTE200',
        'L1_1ZDC_NZDC_TE5_VTE200',
         
        'L1_ZDC_5XOR_TE5_VTE200', 'L1_ZDC_XOR4_TE5_VTE200',
        'L1_5ZDC_A_5ZDC_C_TE5_VTE200', 'L1_VZDC_A_VZDC_C_VTE200',
        'L1_VZDC_A_VZDC_C_TE5', 'L1_ZDC_XOR_TE5',

        'L1_ZDC_A_C_VTE10', 'L1_ZDC_XOR_VTE10',

        'L1_TAU1_VZDC_A_VZDC_C_VTE100', 'L1_TAU1_ZDC_XOR4_VTE100',
        'L1_TAU2_VZDC_A_VZDC_C_VTE100', 'L1_TAU2_ZDC_XOR4_VTE100',
        'L1_TAU1_TRT_VZDC_A_VZDC_C_VTE100', 'L1_TAU1_TRT_ZDC_XOR4_VTE100',
        'L1_TRT_VZDC_A_VZDC_C_VTE50', 'L1_TRT_VZDC_A_VZDC_C_VTE20',
        'L1_TRT_VZDC_A_VZDC_C_VTE200',

        'L1_ZDC_A_C_VTE50',
        'L1_ZDC_A_C_VTE50_OVERLAY',
        'L1_ZDC_XOR4_VTE200', 'L1_VZDC_A_VZDC_C_VTE50',

        'L1_eEM1_VZDC_A_VZDC_C_VTE100', 'L1_eEM1_ZDC_XOR4_VTE100',
        'L1_eEM2_VZDC_A_VZDC_C_VTE100', 'L1_eEM2_ZDC_XOR4_VTE100',

        # L1 items for 2022 Nov. heavy ion test run, ATR-26405
        # Additionla peripheral physics L1 items
        'L1_VTE5', 
        'L1_MBTS_1_VTE5', 
        # Additioanl supporting itesm for BeamSpot, IDCalib
        #'L1_J12_VTE100',
        #'L1_J30_VTE200',
        'L1_J100_VTE200', # to be checked if J100 is too high
        'L1_XE35_VTE200',
        'L1_XE50_VTE200',

        #Topo
        'L1_LAR-ZEE', 
 
    ]

    # To replace thresholds in the physics menu
    # Do not use for L1Topo decision threshold!
    L1MenuFlags.ThresholdMap = {
        #example: 'jXE100' :'',
    }

    # To replace items in the physics menu
    L1MenuFlags.ItemMap = {


        # Others
        'L1_J400_LAR':'',
        'L1_jJ500_LAR':'',

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
        'L1_AFP_A_OR_C_MBTS_2':'',
        'L1_AFP_A_AND_C_MBTS_2':'',
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

