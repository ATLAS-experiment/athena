# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from . import Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds


def defineInputsMenu():

    phys_menu_inputs.defineInputsMenu()

    # For now this reproduces the pp menu exactly
    # In case changes are needed, the example of the MC menu inputs
    # can be followed in order to overwrite the contents to be modified

    #----------------------------------------------

    remapThresholds(L1MenuFlags)

