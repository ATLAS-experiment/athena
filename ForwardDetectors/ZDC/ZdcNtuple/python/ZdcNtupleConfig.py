# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

def ZdcNtupleCfg(flags, name="ZdcNtuple", **kwargs):
    acc = ComponentAccumulator()

    if "TrigDecisionTool" not in kwargs and not flags.Input.isMC:
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("TrigDecisionTool", acc.getPrimaryAndMerge(
            TrigDecisionToolCfg(flags)))

    if "TrackSelectionTool" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_LoosePrimary_Cfg
        kwargs.setdefault("TrackSelectionTool", acc.getPrimaryAndMerge(
            InDetTrackSelectionTool_LoosePrimary_Cfg(flags, maxZ0SinTheta=1.5, maxD0overSigmaD0=3)))

    alg = CompFactory.ZdcNtuple(name, **kwargs)


    acc.addEventAlgo(alg)
    
    return acc

if __name__ == '__main__':

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    acc.merge(ZdcNtupleCfg(flags,
                           name = "AnalysisAlg",
                           zdcConfig = "LHCf2022",
                           lhcf2022 = False,
                           lhcf2022zdc = True,
                           lhcf2022afp = False,
                           zdcOnly = False,
                           useGRL = False,
                           zdcCalib = False,
                           reprocZdc = False,
                           enableOutputTree = True,
                           enableOutputSamples = False,
                           enableTrigger = True,
                           enableTracks = True,
                           enableClusters = True,
                           writeOnlyTriggers = True)) 


    acc.addService(CompFactory.THistSvc(Output = ["ANALYSIS DATAFILE='NTUP.root' OPT='RECREATE'"]))

    acc.printConfig(withDetails=True)
    
    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    import sys
    sys.exit(status.isFailure())
