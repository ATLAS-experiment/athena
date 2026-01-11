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

        alg = GlobalMultiplicityAlgo( name = 'eEMall', input = 'eEM', output = 'eEMall' )
        # alg.addgeneric('InputWidth', HW.eEmInputWidth)
        # alg.addgeneric('OutputWidth', HW.eEmInputWidth)
        # alg.addvariable('REtaMin',   0)
        # alg.addvariable('RHadMin',   0)
        # alg.addvariable('WsTotMin',  0)
        tm.registerTopoAlgo(alg)  


