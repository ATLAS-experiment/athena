# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef
import TriggerMenuMT.L1.Menu.Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs


def defineInputsMenu():
    
    phys_menu_inputs.defineInputsMenu()

    for boardName, boardDef in L1MenuFlags.boards().items():
        if "connectors" in boardDef:
            for conn in boardDef["connectors"]:

                # Add more multiplicity inputs

                # Topo1Opt3
                if conn["name"] == "Topo1Opt3":
                    conn["thresholds"] += [
                        ('jXEPerf100',1),
<<<<<<< HEAD
                    ]
=======
                    ]              
>>>>>>> ceee5aad679 (reverting accidental changes...)
                if conn["name"] == "Topo1Opt1":
                    conn["thresholds"] += [
                        ('eTAU40HT',2),
                    ]
                if conn["name"] == "Topo1Opt1":
                    conn["thresholds"] += [
                        ('eTAU60HM',2),
                    ]
                if conn["name"] == "Topo1Opt1":
                    conn["thresholds"] += [
                        ('eTAU60HL',2),
                    ]
                if conn["name"] == "Topo1Opt1":
                    conn["thresholds"] += [
                        ('eTAU80HL',2),
                    ]
=======
                    ]              

>>>>>>> 82487557f03 (rebase to 24.0)
                # Add more decision algorithms
                if conn["name"] == "Topo2El":
                    for group in conn["algorithmGroups"]:
                        if group["fpga"]==0 and group["clock"]==1:
                            group["algorithms"] += [
                                    TopoMenuDef( '0DR04-MU5VFab-CjJ90ab',     outputbits = 11), #Bjet, TODO: not a primary
                                    TopoMenuDef( '2DISAMB-jJ55ab-4DR28-eTAU30ab-eTAU20ab',  outputbits = 13),
                                    TopoMenuDef( '2DISAMB-jJ55ab-4DR32-eTAU30ab-eTAU20ab',  outputbits = 14),
                                    TopoMenuDef( '2DISAMB-jJ55ab-10DR32-eTAU30ab-eTAU20ab', outputbits = 15),
                                    TopoMenuDef( '0DETA24-4DPHI99-eTAU30ab-eTAU20ab',       outputbits = 16),
                                    TopoMenuDef( '0DETA24-10DPHI99-eTAU30ab-eTAU12ab',      outputbits = 17),
                            ]
                        elif group["fpga"]==1 and group["clock"]==0:
                            group["algorithms"] += [
                                    TopoMenuDef( 'INVM_BOOSTDR_eEMsl6',                       outputbits = (10,11), outputlines = ['0INVM70-2DR15-eEM9sl1-eEM9sl6', 
                                                                                                                                   '0INVM70-2DR15-eEM12sl1-eEM12sl6']),   
                                                                                                                     
                                    TopoMenuDef( 'INVM_BOOSTDR_Ranges_Asymm_eEMsl6',                outputbits = (12,13), outputlines = ['0INVM30-2DR15-eEM12sl1-eEM9sl6', 
                                                                                                                                   '25INVM70-13DR25-eEM12sl1-eEM9sl6']),   
                            ]

    #----------------------------------------------

    def remapThresholds():
        # remap thresholds. TODO: add checks in case the remap does not fulfill HW constraints?
        for boardName, boardDef in L1MenuFlags.boards().items():
            if "connectors" in boardDef:
                for conn in boardDef["connectors"]:
                    if "thresholds" in conn:
                        thresholdsToRemove = []
                        for thrIndex, thrName in enumerate(conn["thresholds"]):
                            nBits = 0
                            if type(thrName)==tuple:
                                (thrName,nBits) = thrName
                            if thrName in L1MenuFlags.ThresholdMap():
                                if (L1MenuFlags.ThresholdMap()[thrName] != ''):
                                    if nBits > 0:
                                        conn["thresholds"][thrIndex] = (L1MenuFlags.ThresholdMap()[thrName],nBits)
                                    else:
                                        conn["thresholds"][thrIndex] = L1MenuFlags.ThresholdMap()[thrName]
                                else:
                                    thresholdsToRemove.append(thrIndex) 
                        for i in reversed(thresholdsToRemove):
                            del conn["thresholds"][i]

    #----------------------------------------------

    remapThresholds()

