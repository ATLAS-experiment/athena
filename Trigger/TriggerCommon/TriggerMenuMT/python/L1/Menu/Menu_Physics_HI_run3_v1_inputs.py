# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from ..Base.MenuConfObj import TopoMenuDef
from ..Base.L1MenuFlags import L1MenuFlags
from . import Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs
from .Menu_Physics_pp_run3_v1_inputs import remapThresholds


def defineInputsMenu():

    phys_menu_inputs.defineInputsMenu()

    # Add HI exclusive decision algorithms on top of the existing ones
    for boardName, boardDef in L1MenuFlags.boards().items():
        if 'connectors' in boardDef:
            for conn in boardDef['connectors']:
                if conn['name'] == 'Topo2El':
                    for group in conn['algorithmGroups']:
                        match group['fpga'], group['clock']:
                            case 0, 0:
                                group['algorithms'] += [
                                    TopoMenuDef('10INVM200-20DPHI32-2jJ5s', outputbits=13),  # ATR-30727
                                    TopoMenuDef('15INVM200-20DPHI32-2jJ5s', outputbits=14),  # ATR-30727
                                    TopoMenuDef('20INVM200-20DPHI32-2jJ5s', outputbits=15),  # ATR-30727
                                ]
                            case 0, 1:
                                group['algorithms'] += [
                                    TopoMenuDef('25INVM200-20DPHI32-2jJ5s', outputbits=13),  # ATR-30727
                                    TopoMenuDef('10SUM200-20DPHI32-2jJ5s',  outputbits=14),  # ATR-30727
                                    TopoMenuDef('15SUM200-20DPHI32-2jJ5s',  outputbits=15),  # ATR-30727
                                ]
                            case 1, 0:
                                group['algorithms'] += [
                                    TopoMenuDef('20SUM200-20DPHI32-2jJ5s',           outputbits=12),  # ATR-30727
                                    TopoMenuDef('25SUM200-20DPHI32-2jJ5s',           outputbits=13),  # ATR-30727
                                    TopoMenuDef('15INVM200-15SUM200-20DPHI32-2jJ5s', outputbits=14),  # ATR-30727
                                    TopoMenuDef('20INVM200-20SUM200-20DPHI32-2jJ5s', outputbits=15),  # ATR-30727
                                ]
                            case _, _:
                                pass
                if conn['name'] == 'Topo3El':
                    for group in conn['algorithmGroups']:
                        match group['fpga'], group['clock']:
                            case 1, 0:
                                group['algorithms'] += [
                                    TopoMenuDef('1INVM200-23DPHI32-2eTAU1s', outputbits=2),  # ATR-30728
                                    TopoMenuDef('2INVM200-23DPHI32-2eTAU1s', outputbits=3),  # ATR-30728
                                    TopoMenuDef('3INVM200-23DPHI32-2eTAU1s', outputbits=4),  # ATR-30728
                                    TopoMenuDef('4INVM200-23DPHI32-2eTAU1s', outputbits=5),  # ATR-30728
                                    TopoMenuDef('3SUM200-23DPHI32-2eTAU1s',  outputbits=6),  # ATR-30728
                                    TopoMenuDef('4SUM200-23DPHI32-2eTAU1s',  outputbits=7),  # ATR-30728
                                ]
                            case _, _:
                                pass

    L1MenuFlags.ThresholdMap = {
        'eTAU70': 'eTAU1',
    }

    remapThresholds(L1MenuFlags)
