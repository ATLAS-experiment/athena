# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# *** IMPORTANT ***
# Menu parameter ordering must match that in the L1Topo
# firmware generation, document this for every algorithm
# and ensure that addvariable order matches
# Refer to https://gitlab.cern.ch/atlas-l1calo/l1topo/ph1topo/-/tree/master/src/algo

# algorithm python base classes generated from C++ code
from ..Base.GlobalAlgos import GlobalMultiplicityAlgo

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class GlobalHypoAlgoDef:

    @staticmethod
    def registerGlobalHypoAlgos(tm):
        GlobalHypoAlgoDef.registerMultiplicityAlgos(tm)

    # Multiplicity algorithms
    @staticmethod
    def registerMultiplicityAlgos(tm):

        multiplicities = {
            "eEM": [('eEM5', 4), 'eEM7',
                    ('eEM9', 3), 'eEM10L', 'eEM12L',
                    ('eEM15', 2), 'eEM18', 'eEM18L', 'eEM18M', 'eEM22M', 'eEM24L'],
            "eTAU": [('eTAU70',3), 'eTAU12', 'eTAU20',
                     ('eTAU20L',2), 'eTAU20M', 'eTAU30', 'eTAU30M', 'eTAU35', 'eTAU35M',
                     'eTAU40HM', 'eTAU40HT', 'eTAU60', 'eTAU60HM', 'eTAU80', 'eTAU140'],
            "WTACone": [("WTACone100", 4), ("WTACone130", 3),
                        ("WTACone160", 2), "WTACone200p0Eta32C", "SPARE"]
        }

        for inputType, thrDefs in multiplicities.items():
            thrDef: tuple[str, int] | str
            nbits = 0
            for thrDef in thrDefs:
                thrName, nbits = thrDef if isinstance(thrDef, tuple) else (thrDef, nbits)
                alg = GlobalMultiplicityAlgo( threshold=thrName, input = inputType, output = thrName, nbits=nbits )
                tm.registerTopoAlgo(alg)
