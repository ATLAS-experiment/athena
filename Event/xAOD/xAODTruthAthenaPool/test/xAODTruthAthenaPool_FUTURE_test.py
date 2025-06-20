#!/usr/bin/env python
"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

import ROOT
from AthenaPoolUtilities.TPCnvTestConfig import TPCnvTest

if __name__ == "__main__":

    # Force-load some dictionaries. Needed to work around ROOT-10940.
    ROOT.xAOD.CaloCluster()

    infile = '/home/jchapman/5clone/run/run_q454/myAOD.pool.root' # 'WorkflowReferences/main/q454/v50/myAOD.pool.root'
    keys = [
        #xAOD::TruthParticleAuxContainer_v2
        'PileUpTruthParticles',
    ]

    TPCnvTest(infile, keys)
