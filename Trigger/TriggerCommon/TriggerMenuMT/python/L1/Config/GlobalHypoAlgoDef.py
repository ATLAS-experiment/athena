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

        # Multiplicity algorithms
        for emThr in ['eEM5', 'eEM7', 'eEM9', 'eEM10L', 'eEM12L', 'eEM15', 'eEM18', 'eEM18L', 
            'eEM18M', 'eEM22M', 'eEM24L']:
            alg = GlobalMultiplicityAlgo( name = f'Mult_{emThr}', input = 'eEM', output = emThr )
            tm.registerTopoAlgo(alg)


