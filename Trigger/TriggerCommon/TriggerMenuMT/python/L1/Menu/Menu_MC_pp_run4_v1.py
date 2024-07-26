# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_MC_pp_run3_v1 as Run3

def defineMenu():

    # reuse based on run3 menu
    Run3.defineMenu()

    l1items = L1MenuFlags.items()

    # add run4 new items
    l1items += [
        'L1_eEM10L_MU8F',
        'L1_2eEM10L',
        'L1_MU5VF_cTAU30M',
        'L1_3jJ40',
    ]

    # recover the ones removed by run3 MC
    def check_and_add(str):
        if str in L1MenuFlags.ItemMap():
            l1items.append(str)

    check_and_add('L1_eEM22M')
    check_and_add('L1_jJ140')
        
    # remove AFP and MBTS items
    discard_list = ["L1_AFP", "L1_MBTS"]
    
    def match_any(str):
        return any([str.startswith(pattern) for pattern in discard_list])

    L1MenuFlags.items = [l1 for l1 in l1items if not match_any(l1)]

