# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from . import Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds


def defineInputsMenu():

    phys_menu_inputs.defineInputsMenu()

    L1MenuFlags.ThresholdMap = {
        'eTAU70': 'eTAU1',
    }

    remapThresholds(L1MenuFlags)

