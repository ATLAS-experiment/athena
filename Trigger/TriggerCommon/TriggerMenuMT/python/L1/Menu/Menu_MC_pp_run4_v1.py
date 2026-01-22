# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_MC_pp_run3_v1 as Run3

def defineMenu():

    # reuse based on run3 menu
    Run3.defineMenu()

    l1items = L1MenuFlags.items()

    # remove AFP and MBTS items, plus any item which are being remapped and which are in the pp menu
    discard_list = ["L1_AFP", "L1_MBTS", "L1_eTAU140", "L1_eTAU35", "L1_3jJ90", "L1_MU14FCH_jJ90", "L1_jJ90", "L1_2jJ90_jXE80", "L1_LATE-MU8F_jJ90"]

    def match_any(item):
        return any([item.startswith(pattern) for pattern in discard_list])

    l1items = [l1 for l1 in l1items if not match_any(l1)]
    existed = set(l1items)

    def try_add(item):
        if item not in existed:
            l1items.append(item)

    def try_recover(item):
        if item in L1MenuFlags.ItemMap():
            l1items.append(item)

    # add run4 new items
    try_add("L1_eEM10L_MU8F")
    try_add("L1_2eEM10L")
    try_add("L1_MU5VF_cTAU30M")
    try_add("L1_3jJ40")
    try_add("L1_eEM20M")            # Remapped from eEM1 (HI menu only)
    try_add("L1_eEM24M")            # Remapped from eEM2 (HI menu only)
    try_add("L1_eEM10L_MU5VF")
    try_add("L1_eTAU120")           # Remapped from eTAU140
    try_add("L1_eTAU28M_2eTAU20M")  # Remapped from eTAU35_2eTAU20M
    try_add("L1_jJ70")              # Remapped from jJ90
    try_add("L1_3jJ70")             # Remapped from 3jJ90
    try_add("L1_2jJ70_jXE80")
    try_add("L1_HT150-jJ50s5pETA32")    
    try_add("L1_MU8VF_2MU8F")
    
    # recover the ones removed by run3 MC
    try_recover("L1_eEM22M")
    try_recover("L1_jJ140")

    L1MenuFlags.items = l1items
