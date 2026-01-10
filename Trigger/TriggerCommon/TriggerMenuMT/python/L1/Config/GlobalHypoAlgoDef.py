# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# *** IMPORTANT ***
# Menu parameter ordering must match that in the L1Topo
# firmware generation, document this for every algorithm
# and ensure that addvariable order matches
# Refer to https://gitlab.cern.ch/atlas-l1calo/l1topo/ph1topo/-/tree/master/src/algo

# algorithm python base classes generated from C++ code
import L1TopoAlgorithms.L1TopoAlgConfig as AlgConf
import L1TopoHardware.L1TopoHardware as HW
from .L1CaloThresholdMapping import get_threshold_cut
from .L1TopoKFMETweights import KFMETweightParameters

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from collections import namedtuple

class GlobalHypoAlgoDef:

    @staticmethod
    def registerGlobalHypoAlgos(tm):

        # eEM inputs
        # ALL
        # Parameter ordering:
        # 1. REtaMin
        # 2. RHadMin
        # 3. WsTotMin
        alg = AlgConf.eEmNoSort( name = 'eEMall', inputs = 'eEmTobs', outputs = 'eEMall' )
        alg.addgeneric('InputWidth', HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmInputWidth)
        alg.addvariable('REtaMin',   0)
        alg.addvariable('RHadMin',   0)
        alg.addvariable('WsTotMin',  0)
        tm.registerTopoAlgo(alg)  


