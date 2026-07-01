# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GlobalJet1AlgToolCfg(
        flags,
        name='GlobalJet1AlgTool',
        gblCellTowersKey = "GlobalCellTowers",
        gblSRJetsKey = "GlobalJet1Jets",
        OutputLevel=None):
    
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)

    wtaConeJetAlgTool = CompFactory.GlobalSim.GlobalJet1AlgTool(name)

    if OutputLevel is not None:
        wtaConeJetAlgTool.OutputLevel = OutputLevel

    wtaConeJetAlgTool.GlobalCellTowersKey = gblCellTowersKey
    wtaConeJetAlgTool.GlobalJet1JetsKey = gblSRJetsKey

    alg.globalsim_algs = [wtaConeJetAlgTool]
    #alg.enableDumps = dump

    cfg.addEventAlgo(alg)

    return cfg
    
