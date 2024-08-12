# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# ALL CHANGES TO THE RUN 4 MENU SHOULD BE MADE TO RELEASE 25 OR GREATER
#

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_MC_pp_run3_v1 as Run3

def defineMenu():

    # reuse based on run3 menu
    Run3.defineMenu()

    l1items = L1MenuFlags.items()

    L1MenuFlags.items = l1items
