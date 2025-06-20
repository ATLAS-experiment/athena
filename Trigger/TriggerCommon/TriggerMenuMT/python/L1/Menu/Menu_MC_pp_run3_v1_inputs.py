# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef
from . import Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds


def defineInputsMenu():
    
    phys_menu_inputs.defineInputsMenu()

    for boardName, boardDef in L1MenuFlags.boards().items():
        if "connectors" in boardDef:
            for conn in boardDef["connectors"]:

                # Add more multiplicity inputs

                if conn["name"] == "Topo1Opt1":
                    conn["thresholds"] += [
                        ('eTAU60HL',2), ('eTAU80HL',2),
                    ]
                if conn["name"] == "Topo1Opt3":
                    conn["thresholds"] += [
                        ('jXEPerf100',1),
                    ]

                # Add more decision algorithms
                if conn["name"] == "Topo2El":
                    for group in conn["algorithmGroups"]:
                        if group["fpga"]==0 and group["clock"]==1:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DR04-MU5VFab-CjJ20ab' , outputbits = 14 ),
                                    TopoMenuDef( '0DR04-MU3VFab-CjJ20ab' , outputbits = 15 ),
                            ]
                        elif group["fpga"]==1 and group["clock"]==1:
                            group["algorithms"] += [
                            ]
                if conn["name"] == "Topo3El":
                    for group in conn["algorithmGroups"]:
                        if group['fpga']==1 and group['clock']==0:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DETA24-4DPHI99-eTAU30ab-eTAU20ab',  outputbits = 10 ),
                                    TopoMenuDef( '0DETA24-10DPHI99-eTAU30ab-eTAU12ab', outputbits = 11 ),
                                    TopoMenuDef( '0DR28-eTAU30abm-eTAU20abm',          outputbits = 12 ),
                            ]
                        elif group["fpga"]==1 and group["clock"]==1:
                            group["algorithms"] += [
                            ]
    #----------------------------------------------

    remapThresholds(L1MenuFlags)

