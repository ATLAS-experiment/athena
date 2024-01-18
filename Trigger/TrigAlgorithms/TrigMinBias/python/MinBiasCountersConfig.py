# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDM import recordable

@AccumulatorCache
def SPCounterRecoAlgCfg(flags):
    acc = ComponentAccumulator()
    from TrigMinBias.TrigMinBiasMonitoring import SpCountMonitoring
    alg = CompFactory.TrigCountSpacePoints( SpacePointsKey = recordable("HLT_SpacePointCounts"), 
                                            MonTool = SpCountMonitoring(flags) )
    acc.addEventAlgo(alg)
    return acc

def TrackCounterHypoAlgCfg(flags):
    """"""
    acc = ComponentAccumulator()
    from TrigMinBias.TrigMinBiasMonitoring import TrackCountMonitoring
    # TODO we should get that from the flags
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    idTrigConfig = getInDetTrigConfig('minBias')
    alg = CompFactory.TrackCountHypoAlg(tracksKey=recordable(idTrigConfig.tracks_IDTrig()),
                                        trackCountKey = recordable("HLT_TrackCount"))
    alg.MonTool = TrackCountMonitoring(flags, alg) # monitoring tool configures itself using config of the hypo alg
    acc.addEventAlgo(alg)
    return acc


if __name__ == '__main__':
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior=1
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files=defaultTestFiles.RAW_RUN2 # or ESD or AOD or ...
    flags.lock()

    acc = ComponentAccumulator()
    acc.merge(SPCounterRecoAlgCfg(flags))
    acc.merge(TrackCounterHypoAlgCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged()
