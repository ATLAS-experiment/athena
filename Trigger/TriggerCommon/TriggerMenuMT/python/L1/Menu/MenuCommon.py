# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.Limits import Limits

def print_available(L1MenuFlags):
    import logging
    available = list(set(range(Limits.MaxTrigItems-3)) - set(L1MenuFlags.CtpIdMap.value.values()) - set([508]))
    freeItems = Limits.MaxTrigItems - len(L1MenuFlags.items.value) # correct for ZB and CALREQ items
    floatingItems = sorted(list(set(L1MenuFlags.items.value) - set(L1MenuFlags.CtpIdMap.value.keys()))) # these items get their CTPID assigned automatically
    unusedItemsWithCTPID = set(L1MenuFlags.CtpIdMap.value.keys()) - set(L1MenuFlags.items.value) # this should be empty, otherwise remove the items from the CtpIdMap
    available.sort()
    logging.info("There are %d available CTP IDs: %s", len(available), ",".join(map(str,available)))
    logging.info("IDs >= 472 go in partition 2, IDs >= 492 go in partition 3")
    logging.info("There are %d free items", freeItems)
    logging.info("There are %d floating items: %s", len(floatingItems), ",".join(map(str,floatingItems)))
    logging.info("There are %d unused items with CTP ID: %s", len(unusedItemsWithCTPID), ",".join(map(str,unusedItemsWithCTPID)))

def defineCommonL1Flags(L1MenuFlags):
    L1MenuFlags.CTPVersion = 4 # new CTP

    L1MenuFlags.BunchGroupPartitioning = [1, 15, 15] # partition 1: 1-10, partition 2: empty (was 14), partition 3: 15 (note that BGRP0 is used by all items)
    L1MenuFlags.BunchGroupNames = ['BCRVeto', 'Paired', 'CalReq', 'Empty',
                                   'IsolatedUnpaired', 'NonIsolatedUnpaired', 'EmptyAfterPaired', 'InTrain',
                                   'AbortGapNotCalReq', 'VdM', 'ALFA', 'EmptyBeforePaired',
                                   'EmptyAndPaired']

    L1MenuFlags.MenuPartitioning = [0, 472, 492] # partition 1: ctpid 0-471, partition 2: ctpid 472-491, partition 3: ctpid 492-511


# Define here the list of triggers that should be in all L1 menus
RequiredL1Items = [
        #CALREQ
        'L1_CALREQ0',
        'L1_CALREQ1',
        'L1_CALREQ2',

        # BPTX -- important for CTP timing checks
        'L1_BPTX0_BGRP12','L1_BPTX1_BGRP12',

        # NSW Monitoring
        'L1_NSW_MONITOR',

        # TRT
        'L1_TRT_FILLED', 'L1_TRT_EMPTY',

        # TGC
        'L1_TGC_BURST',
        
        # BCM
        'L1_BCM_Wide', 'L1_BCM_Wide_CALIB', 'L1_BCM_Wide_EMPTY', 'L1_BCM_Wide_BGRP12', 'L1_BCM_Wide_UNPAIREDB1', 'L1_BCM_Wide_UNPAIREDB2', 
        'L1_BCM_2A_2C_UNPAIRED_ISO', 'L1_BCM_2A_2C_BGRP12',
        'L1_BCM_2A_CALIB', 'L1_BCM_2C_CALIB',
        'L1_BCM_2A_EMPTY', 'L1_BCM_2C_EMPTY',
        'L1_BCM_2A_UNPAIREDB1', 'L1_BCM_2A_UNPAIREDB2', 'L1_BCM_2C_UNPAIREDB1', 'L1_BCM_2C_UNPAIREDB2',
        'L1_BCM_2A_FIRSTINTRAIN', 'L1_BCM_2C_FIRSTINTRAIN',
            
        # RNDM
        'L1_RD0_FILLED', 'L1_RD0_UNPAIRED_ISO',  'L1_RD0_EMPTY',
        'L1_RD0_FIRSTEMPTY', 'L1_RD0_BGRP10', 'L1_RD0_BGRP11',
        'L1_RD0_BGRP7', 'L1_RD0_BGRP15',
        'L1_RD0_FIRSTINTRAIN',
        'L1_RD1_EMPTY',
        'L1_RD1_FILLED',
        'L1_RD2_EMPTY',
        'L1_RD2_FILLED',
        'L1_RD3_EMPTY',
        'L1_RD3_FILLED',

         # ZB 
        'L1_ZeroBias', 
        

]

FixedIDMap = {
        # to be used to hardcode CTP IDs for specific items
        # NB: 509-511 for the CALREQ triggers (at the moment, ATR-22654)

        # High-frequency counters fixed to consecutive CTP IDs
        # 8 items with the high frequency per-bunch monitoring counters (HF:111)
        # should be in consecutive cpid, starting a ctpid number with ctpid%16 = 0
        # ATR-23836
        "L1_BCM_2A_FIRSTINTRAIN":480,
        "L1_BCM_2C_FIRSTINTRAIN":481,
        "L1_jJ30":482,
        "L1_MBTS_1":483,
        "L1_MBTS_2":484,
        "L1_MBTS_1_1":485,
        "L1_BCM_Wide":486,
        # "":487,
    }
