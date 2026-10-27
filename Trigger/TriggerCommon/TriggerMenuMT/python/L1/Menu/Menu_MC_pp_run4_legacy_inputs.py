# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds
from . import Menu_MC_pp_run3_v1_inputs as Run3

def defineInputsMenu():
    # reuse based on run3 menu
    Run3.defineInputsMenu()

    L1MenuFlags.ThresholdMap = {
        'eEM1': 'eEM20M',
        'eEM2': 'eEM24M',
        'eTAU140': 'eTAU120',
        'eTAU35' : 'eTAU28M',
        'jJ90' : 'jJ70',
    }
    remapThresholds(L1MenuFlags)

