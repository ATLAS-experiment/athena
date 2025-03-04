# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
                        if group["fpga"]==0 and group["clock"]==0:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DR04-MU5VFab-CjJ40ab'                 , outputbits = 13),
                                    TopoMenuDef( '0DR04-MU3VFab-CjJ20ab'                 , outputbits = 14),
                                    TopoMenuDef( '0DR04-MU5VFab-CjJ20ab'                 , outputbits = 15),
                            ]
                        elif group["fpga"]==0 and group["clock"]==1:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DR04-MU5VFab-CjJ90ab', outputbits = 13), #Bjet, TODO: not a primary
                                    TopoMenuDef( '0DR04-MU8Fab-CjJ30ab' , outputbits = 14),
                                    TopoMenuDef( '0DR04-MU8Fab-CjJ40ab' , outputbits = 15),
                            ]
                        elif group["fpga"]==1 and group["clock"]==0:
                            group["algorithms"] += [
                                    TopoMenuDef( 'INVM_BOOSTDR_eEMsl6',                       outputbits = (12,13), outputlines = ['0INVM70-2DR15-eEM9sl1-eEM9sl6', 
                                                                                                                                   '0INVM70-2DR15-eEM12sl1-eEM12sl6']),   
                                    TopoMenuDef( 'INVM_BOOSTDR_Ranges_Asymm_eEMsl6',          outputbits = (14,15), outputlines = ['0INVM30-2DR15-eEM12sl1-eEM9sl6', 
                                                                                                                                   '25INVM70-13DR25-eEM12sl1-eEM9sl6']),
                            ]
                if conn["name"] == "Topo3El":
                    for group in conn["algorithmGroups"]:
                        if group["fpga"]==0 and group["clock"]==1:
                            group["algorithms"] += [
                                    TopoMenuDef( '2DISAMB_jJ50ab_DR_eTAU_eTAU',             outputbits = (12), outputlines = ['2DISAMB-jJ50ab-0DR28-eTAU30ab-eTAU20ab']),
                                    TopoMenuDef( '2DISAMB_jJ40ab_DR_eTAU_eTAU',             outputbits = (13), outputlines = ['2DISAMB-jJ40ab-0DR28-eTAU30ab-eTAU20ab']),
                                    TopoMenuDef( '2DISAMB_jJ30ab_DR_eTAU_eTAU',             outputbits = (14), outputlines = ['2DISAMB-jJ30ab-0DR28-eTAU30ab-eTAU20ab']),
                            ]
                        elif group['fpga']==1 and group['clock']==0:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DETA24-4DPHI99-eTAU30ab-eTAU20ab',  outputbits = 2),
                                    TopoMenuDef( '0DETA24-10DPHI99-eTAU30ab-eTAU12ab', outputbits = 3),
                            ]
    #----------------------------------------------

    remapThresholds(L1MenuFlags)

