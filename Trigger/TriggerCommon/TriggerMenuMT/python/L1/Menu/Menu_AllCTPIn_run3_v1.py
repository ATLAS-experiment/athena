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
    logging.info("There are %d free items", freeItems)
    logging.info("There are %d floating items: %s", len(floatingItems), ",".join(map(str,floatingItems)))
    logging.info("There are %d unused items with CTP ID: %s", len(unusedItemsWithCTPID), ",".join(map(str,unusedItemsWithCTPID)))


def defineMenu():

    L1MenuFlags.CTPVersion = 4 # new CTP

    L1MenuFlags.BunchGroupPartitioning = [1, 15, 15] # partition 1: 1-10, partition 2: empty (was 14), partition 3: 15 (note that BGRP0 is used by all items)

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

    ]


if __name__ == "__main__": print_available()


