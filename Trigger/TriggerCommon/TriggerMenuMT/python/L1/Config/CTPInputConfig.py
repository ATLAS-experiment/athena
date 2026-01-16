# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class CTPInputConfig:
    """
    Defines the cabling of the CTP input
    https://twiki.cern.ch/twiki/bin/view/Atlas/LevelOneCentralTriggerSetup#CTP_inputs
    """

    @staticmethod
    def cablingLayout():
        inputLayout = {
            "optical": {
                "connector0": "MuCTPiOpt0",
                "connector1": "Topo1Opt0",
                "connector2": "Topo1Opt1",
                "connector3": "Topo1Opt2",
                "connector4": "Topo1Opt3"
            },
            "electrical": {
                "connector0": "Topo3El",
                "connector1": "LegacyTopoMerged",
                "connector2": "Topo2El"
            },
            "ctpin": {
                "slot7": {
                    "connector0": "EM1",
                    "connector1": "EM2",
                    "connector2": "TAU1",
                    "connector3": "TAU2"
                },
                "slot8": {
                    "connector0": "JET1",
                    "connector1": "JET2",
                    "connector2": "EN1",
                    "connector3": "EN2"
                },
                "slot9": {
                    "connector0": "",
                    "connector1": "CTPCAL",
                    "connector2": "NIM1",
                    "connector3": "NIM2"
                }
            }
        }
        return inputLayout

    @staticmethod
    def cablingLayoutRun4():
        inputLayout = {
            "optical": {
                "connector0": "MuCTPiOpt0",
                "connector1": "L0Global",
            },
            "ctpin": {
                "slot9": {
                    "connector0": "",
                    "connector1": "CTPCAL",
                    "connector2": "NIM1",
                    "connector3": "NIM2"
                }
            }
        }
        return inputLayout
