# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Base.Limits import Limits

def print_available():
    import logging
    defineMenu()
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


def defineMenu():

    L1MenuFlags.CTPVersion = 4 # new CTP

    L1MenuFlags.BunchGroupPartitioning = [1, 15, 15] # partition 1: 1-10, partition 2: empty (was 14), partition 3: 15 (note that BGRP0 is used by all items)
    L1MenuFlags.BunchGroupNames = ['BCRVeto', 'Paired', 'CalReq', 'Empty', 
                                   'IsolatedUnpaired', 'NonIsolatedUnpaired', 'EmptyAfterPaired', 'InTrain', 
                                   'AbortGapNotCalReq', 'VdM', 'ALFA', 'EmptyBeforePaired',
                                   'EmptyAndPaired']

    L1MenuFlags.MenuPartitioning = [0, 472, 492] # partition 1: ctpid 0-471, partition 2: ctpid 472-491, partition 3: ctpid 492-511

    # Define one item per CTP connector
    L1MenuFlags.items = [
        # Direct CTP inputs
        'L1_CTPCAL_Thresholds',
        'L1_NIM1_Thresholds',
        'L1_NIM2_Thresholds',

        # Legacy connectors
        'L1_EM1_Thresholds',
        'L1_EM2_Thresholds',
        #
        'L1_TAU1_Thresholds',
        'L1_TAU2_Thresholds',
        #
        'L1_JET1_Thresholds',
        'L1_JET2_Thresholds',
        #
        'L1_EN1_Thresholds',
        'L1_EN2_Thresholds',

        # Require ZeroBiasA/B threshold
        'L1_ZeroBias',
    ]

    # CTP ID 509-511 are reserved for CALREQ
    L1MenuFlags.CtpIdMap = {
        # to be used to hardcode CTP IDs for specific items
        # NB: 509-511 for the CALREQ triggers (at the moment, ATR-22654)

        # High-frequency counters fixed to consecutive CTP IDs
        # 8 items with the high frequency per-bunch monitoring counters (HF:111)
        # should be in consecutive cpid, starting a ctpid number with ctpid%16 = 0
        # ATR-23836
        "L1_BCM_2A_FIRSTINTRAIN":480,
        "L1_BCM_2C_FIRSTINTRAIN":481,
        "L1_J12":482,
        "L1_MBTS_1":483,
        "L1_MBTS_2":484,
        "L1_MBTS_1_1":485,
        "L1_BCM_Wide":486,
        # "":487,
    }


if __name__ == "__main__": print_available()


