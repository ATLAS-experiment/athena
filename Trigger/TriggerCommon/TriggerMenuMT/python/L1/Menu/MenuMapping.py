# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
This map specifies with menu to load from TriggerMenuMT/python/L1/Menu

The mapping takes precedence over the existence of the menu definition file in the above directory
The resolved name is also being used in the L1/Config/ItemDef.py and L1/Config/ThresholdDef*.py files

Prescale sets will be stripped from the menu name
"""

menuMap = {
    # pp
    "Physics_pp_run3_v1"                        : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],
    "PhysicsP1_pp_run3_v1"                      : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],
    "MC_pp_run3_v1"                             : ["MC_pp_run3_v1",     "MC_pp_run3_v1_inputs",     "Physics_pp_run3_v1_inputs_legacy"],
    "Dev_pp_run3_v1"                            : ["MC_pp_run3_v1",     "MC_pp_run3_v1_inputs",     "Physics_pp_run3_v1_inputs_legacy"],

    # pp for run 4
    "Physics_pp_run4_v1"                        : ["Physics_pp_run4_v1","Physics_pp_run4_v1_inputs","Physics_pp_run4_v1_inputs_legacy"],
    #"PhysicsP1_pp_run4_v1"                      : ["Physics_pp_run4_v1","Physics_pp_run4_v1_inputs","Physics_pp_run4_v1_inputs_legacy"],
    "MC_pp_run4_v1"                             : ["MC_pp_run4_v1",     "MC_pp_run4_v1_inputs",     "Physics_pp_run4_v1_inputs_legacy"],
    "Dev_pp_run4_v1"                            : ["MC_pp_run4_v1",     "MC_pp_run4_v1_inputs",     "Physics_pp_run4_v1_inputs_legacy"],

    # low mu
    "PhysicsP1_pp_lowMu_run3_v1"                : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "Dev_pp_lowMu_run3_v1"                      : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],

    # cosmics
    "Cosmic_run3_v1"                            : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],

    # HI
    "PhysicsP1_HI_run3_v1"                      : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "MC_HI_run3_v1"                             : ["MC_HI_run3_v1",      "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "Dev_HI_run3_v1"                            : ["MC_HI_run3_v1",      "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],

    # Special dummy menus needed for compiling CTPIN switch matrix
    "AllCTPIn_pp_run3_v1"                       : ["AllCTPIn_run3_v1",  "Physics_pp_run3_v1_inputs", "Physics_pp_run3_v1_inputs_legacy"],
    "AllCTPIn_HI_run3_v1"                       : ["AllCTPIn_run3_v1",  "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
}
