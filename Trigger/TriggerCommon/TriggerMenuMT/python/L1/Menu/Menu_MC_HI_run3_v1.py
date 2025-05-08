# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_Physics_HI_run3_v1 as physics_menu

def defineMenu():
    physics_menu.defineMenu()

    # Add new items to the item list in the Physics menu
    l1items = L1MenuFlags.items()
    l1items += [
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

