# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from . import Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds


def defineInputsMenu():

    phys_menu_inputs.defineInputsMenu()

    # Please consult with the menu coordinators in case you want to add any algorithms here. If possible all algorithms should be added to Menu_Physics_pp_run3_v1_inputs to keep the L1 menu inputs for pp and HI unified and allow using the same L1Topo FW 

    
    L1MenuFlags.ThresholdMap = {
        'eTAU70': 'eTAU1',
        'eTAU20': 'eTAU2',
    }

    remapThresholds(L1MenuFlags)
