# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef
import TriggerMenuMT.L1.Menu.Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs


def defineInputsMenu():
    phys_menu_inputs.defineInputsMenu()

    for conn in L1MenuFlags.boards()['Topo3']['connectors']:
        if conn['name'] == 'Topo3El':
            for group in conn['algorithmGroups']:
                if group['fpga'] == 0 and group['clock'] == 0:
                    group['algorithms'] += [
                        TopoMenuDef('23DPHI32-2eEM1s', outputbits=13), #ATR-29784
                        TopoMenuDef('23DPHI32-2eTAU1s', outputbits=14), #ATR-29784
                        TopoMenuDef('23DPHI32-2jTAU1s', outputbits=15), #ATR-29784
                    ]

    #----------------------------------------------

    def remapThresholds():
        # remap thresholds. TODO: add checks in case the remap does not fulfill HW constraints?
        for boardName, boardDef in L1MenuFlags.boards().items():
            if "connectors" in boardDef:
                for c in boardDef["connectors"]:
                    if "thresholds" in c:
                        thresholdsToRemove = []
                        for thrIndex, thrName in enumerate(c["thresholds"]):
                            nBits = 0
                            if type(thrName)==tuple:
                                (thrName,nBits) = thrName
                            if thrName in L1MenuFlags.ThresholdMap():
                                if (L1MenuFlags.ThresholdMap()[thrName] != ''):
                                    if nBits > 0:
                                        c["thresholds"][thrIndex] = (L1MenuFlags.ThresholdMap()[thrName],nBits)
                                    else:
                                        c["thresholds"][thrIndex] = L1MenuFlags.ThresholdMap()[thrName]
                                else:
                                    thresholdsToRemove.append(thrIndex)
                        for i in reversed(thresholdsToRemove):
                            del c["thresholds"][i]

    #----------------------------------------------

    remapThresholds()

