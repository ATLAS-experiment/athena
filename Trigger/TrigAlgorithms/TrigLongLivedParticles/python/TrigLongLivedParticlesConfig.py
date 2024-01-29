# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigEDMConfig.TriggerEDM import recordable

def MuonClusterConfig(flags, name="MuonClusterConfig"):
    from TrigLongLivedParticles.TrigLongLivedParticlesMonitoring import trigMuonClusterAlgorithmMonitoring
    acc = ComponentAccumulator()
    alg = CompFactory.MuonCluster(
        name,
        TrigRoIs_CompositeContainer = recordable("HLT_MuRoICluster_Composites"),
        DeltaR      = 0.4,  # muClu Parameters
        MonTool = trigMuonClusterAlgorithmMonitoring(flags))
    acc.addEventAlgo(alg)
    return acc
