# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuSAVtxFitterToolConfig(flags, name="MuSAVtxFitterTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags, IterationNumber = 100, allowUltraDisplaced = True)))


    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))


    kwargs.setdefault("etaCutMSTP", 2.5)
    kwargs.setdefault("baseChi2Cut", 50)

    acc.setPrivateTools(CompFactory.Rec.MuSAVtxFitterTool(name,**kwargs))
    return acc

def MuSAVtxFitterConfig(flags, name="MuSAVtxFitter", **kwargs): 
    acc = ComponentAccumulator()

    kwargs.setdefault("MuSAVtxContainerName", "MuSAVertices")
    kwargs.setdefault("MuSAExtrapolatedTracksName", "MuSAExtrapolatedTrackParticles")
    kwargs.setdefault("MuonContainerName", "Muons")
    kwargs.setdefault("MSTPContainerName", "MuonSpectrometerTrackParticles")

    if "TrackToVertexTool" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    
    if "MuSAVtxToolName" not in kwargs:
        from MuSAVtxFitter.MuSAVtxFitterConfig import MuSAVtxFitterToolConfig
        kwargs.setdefault("MuSAVtxToolName", acc.popToolsAndMerge(MuSAVtxFitterToolConfig(flags)))
        
    acc.addEventAlgo(CompFactory.Rec.MuSAVtxFitter(name, **kwargs))
        
    return acc

# snippet for running standalone / unit test

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Input.Files = defaultTestFiles.AOD_RUN2_MC
    
    flags.fillFromArgs()
    
    flags.lock()

    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    from AthenaConfiguration.Enums import LHCPeriod
    cfg.merge(LRTMuonMergerAlg(flags,
                                OutputMuonLocation    = "StdWithLRTMuons",
                                CreateViewCollection  = False,
                                overlapStrategy       = 1,
                                UseRun3WP = flags.GeoModel.Run == LHCPeriod.Run3))
    cfg.merge(MuSAVtxFitterConfig(flags, MuonContainerName="StdWithLRTMuons"))
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    cfg.getService("MessageSvc").enableSuppression = True

    # Get maximum events from flags
    evtMax = flags.Exec.MaxEvents if flags.Exec.MaxEvents > 0 else None
    
    sc = cfg.run(evtMax)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")
