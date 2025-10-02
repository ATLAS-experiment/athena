# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
import TriggerMenuMT.L1.Menu.Menu_MC_pp_run3_v1_inputs as Run3
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds

def defineInputsMenu():
    # reuse based on run3 menu
    Run3.defineInputsMenu()

    L1MenuFlags.ThresholdMap = {
        'eEM1': 'eEM20M',
        'eEM2': 'eEM24M',
    }
    remapThresholds(L1MenuFlags)

