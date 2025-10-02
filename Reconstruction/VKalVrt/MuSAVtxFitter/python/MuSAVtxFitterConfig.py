# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#default MuSAVtxFitterTool configuration
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
    
    kwargs.setdefault("doValidation", False)

    acc.setPrivateTools(CompFactory.Rec.MuSAVtxFitterTool(name,**kwargs))
    return acc

#default MuSAVtxFitter configuration
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

# config for J/Psi collection for tag and probe validation
def MuSAVtxJPsiValidationAlgCfg(flags, name="MuSAVtxJPsiValidationAlg", **kwargs):
    acc = ComponentAccumulator()

    # Define J/Psi mass window
    Jpsi_lo = 2000  # MeV
    Jpsi_hi = 4000  # MeV

    # Get required tools
    from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
    vkalvrt = acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags))
    
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    
    from InDetConfig.InDetTrackSelectorToolConfig import InDetTrackSelectorToolCfg
    trackselect = acc.popToolsAndMerge(InDetTrackSelectorToolCfg(flags, Extrapolator=extrapolator))
    acc.addPublicTool(trackselect)
    
    from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags))
    acc.addPublicTool(vpest)

    # Configure the JpsiFinder tool
    jpsiFinderTool = CompFactory.Analysis.JpsiFinder(
        name = "JPsiFinderTool",
        muAndMu = True,  # Default: m_mumu = true
        muAndTrack = False,  # Default: m_mutrk = false
        TrackAndTrack = False,  # Default: m_trktrk = false
        assumeDiMuons = True,  # Default: m_diMuons = true
        invMassLower = Jpsi_lo,  # Default: m_invMassLower = 0.0
        invMassUpper = Jpsi_hi,  # Default: m_invMassUpper = 100000.0
        Chi2Cut = 50.,  # Default: m_Chi2Cut = 50.
        oppChargesOnly = True,  # Default: m_oppChOnly = true
        atLeastOneComb = True,  # Default: m_atLeastOneComb = true
        useCombinedMeasurement = False,  # Default: m_useCombMeasurement = false
        muonCollectionKey = "StdWithLRTMuons",  # Default: not explicitly set
        TrackParticleCollection = "InDetWithLRTTrackParticles",  # Default: not explicitly set
        useV0Fitter = False,  # Default: m_useV0Fitter = false
        TrkVertexFitterTool = vkalvrt,  # Default: not explicitly set
        TrackSelectorTool = trackselect,  # Default: not explicitly set
        VertexPointEstimator = vpest,  # Default: not explicitly set
        useMCPCuts = False  # Default: m_mcpCuts = true
    )
    
    acc.addPublicTool(jpsiFinderTool)
    kwargs.setdefault("JpsiFinderTool", jpsiFinderTool)

    kwargs.setdefault("MuonContainer", "StdWithLRTMuons")
    kwargs.setdefault("EventInfo", "EventInfo")
    kwargs.setdefault("JPsiMuonContainer", "JPsiMuons")
    kwargs.setdefault("JPsiVertexContainer", "JPsiVertices") 

    acc.addEventAlgo(CompFactory.Rec.MuSAVtxJPsiValidationAlg(name, **kwargs))

    return acc

#config for running MuSAFitter on all MSTPs
def MuSAVtxFitterValidationConfig(flags, name="MuSAVtxFitterValidation", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MuSAVtxContainerName", "JPsiMuSAVertices")
    kwargs.setdefault("MuSAExtrapolatedTracksName", "JPsiMuSAExtrapolatedTrackParticles")
    kwargs.setdefault("MuonContainerName", "JPsiMuons")
    kwargs.setdefault("MSTPContainerName", "MuonSpectrometerTrackParticles")

    if "TrackToVertexTool" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    
    if "MuSAVtxToolName" not in kwargs:
        from MuSAVtxFitter.MuSAVtxFitterConfig import MuSAVtxFitterToolConfig
        kwargs.setdefault("MuSAVtxToolName", acc.popToolsAndMerge(MuSAVtxFitterToolConfig(flags, doValidation=True)))
        
    acc.addEventAlgo(CompFactory.Rec.MuSAVtxFitter(name, **kwargs))
        
    return acc

# snippet for running standalone / unit test

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA, configureCondTag
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Input.Files = defaultTestFiles.AOD_RUN2_MC
    configureCondTag(flags)
    flags.fillFromArgs()
    
    flags.lock()

    cfg = SetupMuonStandaloneCA(flags)
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    from AthenaConfiguration.Enums import LHCPeriod
    cfg.merge(LRTMuonMergerAlg(flags,
                                OutputMuonLocation    = "StdWithLRTMuons",
                                CreateViewCollection  = False,
                                overlapStrategy       = 1,
                                UseRun3WP = flags.GeoModel.Run == LHCPeriod.Run3))
    cfg.merge(MuSAVtxFitterConfig(flags, MuonContainerName="StdWithLRTMuons"))
    flags.dump()

    cfg.getService("MessageSvc").enableSuppression = True

    executeTest(cfg)
   