# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY25.py
# Contact: xin.chen@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY25"
streamName = "StreamDAOD_BPHY25"
# mass limits and constants used in the following # TODO Move to TruthUtils?
Jpsi_lo = 2600.0
Jpsi_hi = 3500.0
B_lo = 5080.0
B_hi = 5480.0
Ks_lo = 430.0
Ks_hi = 565.0
Ld_lo = 1030.0
Ld_hi = 1200.0
Xi_lo = 1260.0
Xi_hi = 1383.0
Omg_lo = 1600.0
Omg_hi = 1745.0
Ldb0_lo = 5310.0
Ldb0_hi = 5910.0

Mumass = 105.658
Pimass = 139.570
Kmass = 493.677
Ksmass = 497.611
Jpsimass = 3096.916
B0mass = 5279.66
Lambdamass = 1115.683
Ximass = 1321.71
Omegamass = 1672.45
Lambdab0mass = 5619.60


def BPHY25JpsiXPlusDisplaced_DisVCfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyDisVMassConstrain", True)#
    kwargs.setdefault("ApplyMainVMassConstrain", False)
    kwargs.setdefault("ApplyV0MassConstrain", True)
    kwargs.setdefault("Chi2Cut", 4.)
    kwargs.setdefault("Chi2CutDisV", 4.)
    kwargs.setdefault("Chi2CutGamma", 3.)
    kwargs.setdefault("Chi2CutV0", 4.)
    kwargs.setdefault("DoCascadeFitWithPV", 2)
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    kwargs.setdefault("FirstDecayAtPV", False)#
    kwargs.setdefault("GammaFitterTool", acc.addPublicTool(BPHY25GammaFitterCfg(flags)))
    kwargs.setdefault("HasJXSubVertex", False)#
    kwargs.setdefault("JXDaug1MassHypo", Mumass)
    kwargs.setdefault("JXDaug2MassHypo", Mumass)
    kwargs.setdefault("JXVertices", "BPHY25OniaCandidates")#
    kwargs.setdefault("KsMass", Ksmass)
    kwargs.setdefault("KsMassLowerCut", Ks_lo)
    kwargs.setdefault("KsMassUpperCut", Ks_hi)
    kwargs.setdefault("LambdaMass", Lambdamass)
    kwargs.setdefault("LambdaMassLowerCut", Ld_lo)
    kwargs.setdefault("LambdaMassUpperCut", Ld_hi)
    kwargs.setdefault("LxyDisVtxCut", 2.)#
    kwargs.setdefault("LxyV0Cut", 1.)
    kwargs.setdefault("MassCutGamma", 10.)
    kwargs.setdefault("MassLowerCut", 0.)
    kwargs.setdefault("MassUpperCut", 20000.)#
    kwargs.setdefault("MaxDisVCandidates", 30)
    kwargs.setdefault("MaxJXCandidates", 10)#
    kwargs.setdefault("MaxMainVCandidates", 30)
    kwargs.setdefault("MaxV0Candidates", 20)
    kwargs.setdefault("NumberOfDisVDaughters", 3)#
    kwargs.setdefault("NumberOfJXDaughters", 2)#
    kwargs.setdefault("PVContainerName", "BPHY25_mumuRefittedPrimaryVertices")
    kwargs.setdefault("PVRefitter", acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))))
    kwargs.setdefault("RefitPV", False)
    kwargs.setdefault("RelinkTracks", ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"])
    kwargs.setdefault("Trackd0Cut", 3.0)
    kwargs.setdefault("TrackParticleCollection", "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles")
    kwargs.setdefault("TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("TrackToVertexTool", acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))))
    kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))    kwargs.setdefault("UseImprovedMass", True)
    kwargs.setdefault("V0Hypothesis", "Lambda")#
    kwargs.setdefault("V0Tools", acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("V0TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))))
    kwargs.setdefault("V0VertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))))
    kwargs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
))
    acc.addEventAlgo(CompFactory.DerivationFramework.JpsiXPlusDisplaced(name, **kwargs))
    return acc


def BPHY25JpsiXPlusDisplaced_BpmV0Cfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyJXMassConstraint", True)#
    kwargs.setdefault("ApplyJpsiMassConstraint", True)#
    kwargs.setdefault("ApplyMainVMassConstraint", False)
    kwargs.setdefault("ApplyV0MassConstraint", True)
    kwargs.setdefault("Chi2Cut", 4.)
    kwargs.setdefault("Chi2CutGamma", 3.)
    kwargs.setdefault("Chi2CutV0", 4.)
    kwargs.setdefault("DoCascadeFitWithPV", 2)
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    kwargs.setdefault("FirstDecayAtPV", True)#
    kwargs.setdefault("GammaFitterTool", acc.addPublicTool(BPHY25GammaFitterCfg(flags)))
    kwargs.setdefault("HasJXSubVertex", True)#
    kwargs.setdefault("JXDaug1MassHypo", Mumass)
    kwargs.setdefault("JXDaug2MassHypo", Mumass)
    kwargs.setdefault("JXPtOrdering", False)#
    kwargs.setdefault("JpsiMass", Jpsimass)#
    kwargs.setdefault("JpsiMassLowerCut", Jpsi_lo)#
    kwargs.setdefault("JpsiMassUpperCut", Jpsi_hi)#
    kwargs.setdefault("KsMass", Ksmass)
    kwargs.setdefault("KsMassLowerCut", Ks_lo)
    kwargs.setdefault("KsMassUpperCut", Ks_hi)
    kwargs.setdefault("LambdaMass", Lambdamass)
    kwargs.setdefault("LambdaMassLowerCut", Ld_lo)
    kwargs.setdefault("LambdaMassUpperCut", Ld_hi)
    kwargs.setdefault("LxyV0Cut", 3.)
    kwargs.setdefault("MassCutGamma", 10.)
    kwargs.setdefault("MassLowerCut", 0.)
    kwargs.setdefault("MassUpperCut", 150000.)#
    kwargs.setdefault("MaxJXCandidates", 20)#
    kwargs.setdefault("MaxMainVCandidates", 30)
    kwargs.setdefault("MaxV0Candidates", 20)
    kwargs.setdefault("NumberOfDisVDaughters", 2)#
    kwargs.setdefault("NumberOfJXDaughters", 3)#
    kwargs.setdefault("PVContainerName", "BPHY25_BpmRefittedPrimaryVertices")
    kwargs.setdefault("PVRefitter", acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))))
    kwargs.setdefault("RefitPV", False)
    kwargs.setdefault("RelinkTracks", ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"])
    kwargs.setdefault("Trackd0Cut", 3.0)
    kwargs.setdefault("TrackParticleCollection", "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles")
    kwargs.setdefault("TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("TrackToVertexTool", acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))))
    kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("UseImprovedMass", True)
    kwargs.setdefault("V0Tools", acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("V0VertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))))
    kwargs.setdefault("V0Vertices", "V0Collection")#
    kwargs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
    kwargs.setdefault("V0TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))))
    acc.addEventAlgo(CompFactory.DerivationFramework.JpsiXPlusDisplaced(name, **kwargs))
    return acc


def BPHY25JpsiXPlusDisplaced_B0LdCfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyJXMassConstraint", True)
    kwargs.setdefault("ApplyJpsiMassConstraint", True)
    kwargs.setdefault("ApplyV0MassConstraint", True)
    kwargs.setdefault("ApplyMainVMassConstraint", False)
    kwargs.setdefault("V0Vertices", "V0Collection")
    kwargs.setdefault("UseImprovedMass", True)
    kwargs.setdefault("LambdaMassLowerCut", Ld_lo)
    kwargs.setdefault("LambdaMassUpperCut", Ld_hi)
    kwargs.setdefault("KsMassLowerCut", Ks_lo)
    kwargs.setdefault("KsMassUpperCut", Ks_hi)
    kwargs.setdefault("JpsiMassLowerCut", Jpsi_lo)
    kwargs.setdefault("JpsiMassUpperCut", Jpsi_hi)
    kwargs.setdefault("MassLowerCut", 0.)
    kwargs.setdefault("MassUpperCut", 150000.)
    kwargs.setdefault("HasJXSubVertex", True)
    kwargs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
    kwargs.setdefault("V0Hypothesis", "Lambda")
    kwargs.setdefault("MassCutGamma", 10.)
    kwargs.setdefault("Chi2CutGamma", 3.)
    kwargs.setdefault("LxyV0Cut", 3.)
    kwargs.setdefault("NumberOfJXDaughters", 4)
    kwargs.setdefault("JXDaug1MassHypo", Mumass)
    kwargs.setdefault("JXDaug2MassHypo", Mumass)
    kwargs.setdefault("JXPtOrdering", False)
    kwargs.setdefault("NumberOfDisVDaughters", 2)
    kwargs.setdefault("JpsiMass", Jpsimass)
    kwargs.setdefault("LambdaMass", Lambdamass)
    kwargs.setdefault("KsMass", Ksmass)
    kwargs.setdefault("Chi2CutV0", 4.)
    kwargs.setdefault("Chi2Cut", 4.)
    kwargs.setdefault("Trackd0Cut", 3.0)
    kwargs.setdefault("MaxJXCandidates", 20)
    kwargs.setdefault("MaxV0Candidates", 20)
    kwargs.setdefault("MaxMainVCandidates", 30)
    kwargs.setdefault("RefitPV", False)
    kwargs.setdefault("PVContainerName", "BPHY25_B0RefittedPrimaryVertices")
    kwargs.setdefault("DoCascadeFitWithPV", 2)
    kwargs.setdefault("FirstDecayAtPV", True)
    kwargs.setdefault("TrackParticleCollection", "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles")
    kwargs.setdefault("RelinkTracks", ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"])
    kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("V0VertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))))
    kwargs.setdefault("GammaFitterTool", acc.addPublicTool(BPHY25GammaFitterCfg(flags)))
    kwargs.setdefault("PVRefitter", acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))))
    kwargs.setdefault("V0Tools", acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("TrackToVertexTool", acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))))
    kwargs.setdefault("V0TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))))
    kwargs.setdefault("TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    acc.addEventAlgo(CompFactory.DerivationFramework.JpsiXPlusDisplaced(name, **kwargs))
    return acc


def BPHY25JpsiXPlusDisplaced_B0KsCfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyJXMassConstraint", True)
    kwargs.setdefault("ApplyJpsiMassConstraint", True)
    kwargs.setdefault("ApplyMainVMassConstraint", False)
    kwargs.setdefault("ApplyV0MassConstraint", True)
    kwargs.setdefault("Chi2Cut", 5.)
    kwargs.setdefault("Chi2CutGamma", 3.)
    kwargs.setdefault("Chi2CutV0", 5.)
    kwargs.setdefault("DoCascadeFitWithPV", 2)
    kwargs.setdefault("FirstDecayAtPV", True)
    kwargs.setdefault("HasJXSubVertex", True)
    kwargs.setdefault("JXDaug1MassHypo", Mumass)
    kwargs.setdefault("JXDaug2MassHypo", Mumass)
    kwargs.setdefault("JXPtOrdering", False)
    kwargs.setdefault("JpsiMass", Jpsimass)
    kwargs.setdefault("JpsiMassLowerCut", Jpsi_lo)
    kwargs.setdefault("JpsiMassUpperCut", Jpsi_hi)
    kwargs.setdefault("KsMass", Ksmass)
    kwargs.setdefault("KsMassLowerCut", Ks_lo)
    kwargs.setdefault("KsMassUpperCut", Ks_hi)
    kwargs.setdefault("LambdaMass", Lambdamass)
    kwargs.setdefault("LambdaMassLowerCut", Ld_lo)
    kwargs.setdefault("LambdaMassUpperCut", Ld_hi)
    kwargs.setdefault("LxyV0Cut", 3.)
    kwargs.setdefault("MainVtxMass", 5839.88) # Bs2
    kwargs.setdefault("MassCutGamma", 10.)
    kwargs.setdefault("MassLowerCut", 0.)
    kwargs.setdefault("MassUpperCut", 150000.)
    kwargs.setdefault("MaxJXCandidates", 20)
    kwargs.setdefault("MaxMainVCandidates", 30)
    kwargs.setdefault("MaxV0Candidates", 20)
    kwargs.setdefault("NumberOfDisVDaughters", 2)
    kwargs.setdefault("NumberOfJXDaughters", 4)
    kwargs.setdefault("PVContainerName", "BPHY25_B0RefittedPrimaryVertices")
    kwargs.setdefault("RefitPV", False)
    kwargs.setdefault("Trackd0Cut", 3.0)
    kwargs.setdefault("UseImprovedMass", True)
    kwargs.setdefault("V0Hypothesis", "Ks")
    kwargs.setdefault("V0Vertices", "V0Collection")
    kwargs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
    kwargs.setdefault("TrackParticleCollection", "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles")
    kwargs.setdefault("RelinkTracks", ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"])
    kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("V0VertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))))
    kwargs.setdefault("GammaFitterTool", acc.addPublicTool(BPHY25GammaFitterCfg(flags)))
    kwargs.setdefault("PVRefitter", acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))))
    kwargs.setdefault("V0Tools", acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("TrackToVertexTool", acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))))
    kwargs.setdefault("V0TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))))
    kwargs.setdefault("TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    acc.addEventAlgo(CompFactory.DerivationFramework.JpsiXPlusDisplaced(name, **kwargs))
    return acc


def BPHY25JpsiXPlus2V0Cfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyJpsiMassConstraint", True)
    kwargs.setdefault("ApplyMainVMassConstraint", False)
    kwargs.setdefault("ApplyV01MassConstraint", True)
    kwargs.setdefault("ApplyV02MassConstraint", True)
    kwargs.setdefault("Chi2Cut", 4.)
    kwargs.setdefault("Chi2CutGamma", 3.)
    kwargs.setdefault("Chi2CutV0", 4.)
    kwargs.setdefault("DoCascadeFitWithPV", 2)
    kwargs.setdefault("JXDaug1MassHypo", Mumass)
    kwargs.setdefault("JXDaug2MassHypo", Mumass)
    kwargs.setdefault("JXVertices", "BPHY25OniaCandidates")
    kwargs.setdefault("JXVtxHypoNames", ["Jpsi"])
    kwargs.setdefault("JpsiMass", Jpsimass)
    kwargs.setdefault("JpsiMassLowerCut", Jpsi_lo)
    kwargs.setdefault("JpsiMassUpperCut", Jpsi_hi)
    kwargs.setdefault("KsMass", Ksmass)
    kwargs.setdefault("KsMassLowerCut", Ks_lo)
    kwargs.setdefault("KsMassUpperCut", Ks_hi)
    kwargs.setdefault("LambdaMass", Lambdamass)
    kwargs.setdefault("LambdaMassLowerCut", Ld_lo)
    kwargs.setdefault("LambdaMassUpperCut", Ld_hi)
    kwargs.setdefault("LxyV01Cut", 2.)
    kwargs.setdefault("LxyV02Cut", 2.)
    kwargs.setdefault("MassCutGamma", 10.)
    kwargs.setdefault("MassLowerCut", 0.)
    kwargs.setdefault("MassUpperCut", 150000.)
    kwargs.setdefault("MaxJXCandidates", 10)
    kwargs.setdefault("MaxMainVCandidates", 30)
    kwargs.setdefault("MaxV0Candidates", 20)
    kwargs.setdefault("NumberOfJXDaughters", 2)
    kwargs.setdefault("PVContainerName", "BPHY25_mumuRefittedPrimaryVertices")
    kwargs.setdefault("RefitPV", False)
    kwargs.setdefault("Trackd0Cut", 3.0)
    kwargs.setdefault("UseImprovedMass", True)
    kwargs.setdefault("V01Hypothesis", "Lambda/Ks")
    kwargs.setdefault("V0Vertices", "V0Collection")
    kwargs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
    kwargs.setdefault("TrackParticleCollection", "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles")
    kwargs.setdefault("RelinkTracks", ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"])
    kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("V0VertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))))
    kwargs.setdefault("GammaFitterTool", acc.addPublicTool(BPHY25GammaFitterCfg(flags)))
    kwargs.setdefault("PVRefitter", acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))))
    kwargs.setdefault("V0Tools", acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))))
    kwargs.setdefault("TrackToVertexTool", acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))))
    kwargs.setdefault("V0TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))))
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    acc.addEventAlgo(CompFactory.DerivationFramework.JpsiXPlus2V0(name, **kwargs))
    return acc


def BPHY25JpsiXPlus2V0ACfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("FirstDecayAtPV", False)
    kwargs.setdefault("HasJXSubVertex", False)
    kwargs.setdefault("HasJXV02SubVertex", False)
    kwargs.setdefault("V02Hypothesis", "Lambda/Ks")
    return BPHY25JpsiXPlus2V0Cfg(flags, name, **kwargs)


def BPHY25JpsiXPlus2V0BCfg(flags, name, kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ApplyJXV02MassConstraint", True)
    kwargs.setdefault("FirstDecayAtPV", True)
    kwargs.setdefault("HasJXSubVertex", True)
    kwargs.setdefault("HasJXV02SubVertex", True)
    return BPHY25JpsiXPlus2V0Cfg(flags, name, **kwargs)


def BPHY25GammaFitterCfg(flags, kwargs):
    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg
    return V0VKalVrtFitterCfg(
        flags, BPHYDerivationName+"_GammaFitter",
        Robustness          = 6,
        usePhiCnst          = True,
        useThetaCnst        = True,
        InputParticleMasses = [0.511,0.511] )


def BPHY25_Reco_mumuCfg(flags):
    acc = ComponentAccumulator()
    BPHY25JpsiFinder =  CompFactory.Analysis.JpsiFinder(
        name = "BPHY25JpsiFinder",
        muAndMu = True,
        muAndTrack = False,
        TrackAndTrack = False,
        assumeDiMuons = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt = 3400.,
        invMassLower = 500.,
        invMassUpper = 4500.,
        Chi2Cut = 4., # NDF = 1 if no mass constraint
        oppChargesOnly = True,
        atLeastOneComb = True,
        useCombinedMeasurement = False, # Only takes effect if combOnly = True
        muonCollectionKey = "Muons",
        TrackParticleCollection = "InDetTrackParticles",
        TrkVertexFitterTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))), # VKalVrt vertex fitter
        TrackSelectorTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))),
        VertexPointEstimator = acc.addPublicTool(acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))),
        useMCPCuts = False )
    acc.addPublicTool(BPHY25JpsiFinder) # NOT REQUIRED

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY25_Reco_mumu",
        VertexSearchTool = BPHY25JpsiFinder,# Private tool handle
        OutputVtxContainerName = "BPHY25OniaCandidates",
        PVContainerName = "PrimaryVertices",
        RelinkTracks = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"],
        V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))),
        PVRefitter = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))),
        RefitPV = True,
        MaxPVrefit = 500,
        RefPVContainerName = "BPHY25_mumuRefittedPrimaryVertices",
        DoVertexType = 7))
    return acc


def BPHY25FourTrackReco_B0Cfg(flags):
    acc = ComponentAccumulator()
    # B0 -> J/psi + K pi
    BPHY25B0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name = "BPHY25B0_Jpsi2Trk",
        kaonkaonHypothesis = False,
        pionpionHypothesis = False,
        kaonpionHypothesis = True,
        kaonprotonHypothesis = False,
        trkThresholdPt = 760.,
        trkMaxEta = 2.6,
        oppChargesOnly = False,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkQuadrupletMassLower = B_lo-100,
        TrkQuadrupletMassUpper = B_hi+100,
        BMassLower = B_lo,
        BMassUpper = B_hi,
        DiTrackMassLower = 500.,
        DiTrackMassUpper = 1100.,
        Chi2Cut = 4.,
        JpsiContainerKey = "BPHY25OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))),
        TrackSelectorTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))),
        UseMassConstraint = True)
    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY25FourTrackReco_B0",
        VertexSearchTool = BPHY25B0_Jpsi2Trk, # Private tool handle
        OutputVtxContainerName = "BPHY25FourTrack_B0",
        PVContainerName = "PrimaryVertices",
        RelinkTracks = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"],
        V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))),
        PVRefitter = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))),
        RefitPV = True,
        MaxPVrefit = 500,
        RefPVContainerName = "BPHY25_B0RefittedPrimaryVertices",
        DoVertexType = 7))
    return acc


def BPHY25ThreeTrackReco_BpmCfg(flags):
    acc = ComponentAccumulator()
    # B+ -> J/psi K
    BPHY25Bpm_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name = "BPHY25Bpm_Jpsi1Trk",
        pionHypothesis = False,
        kaonHypothesis = True,
        trkThresholdPt = 950.,
        trkMaxEta = 2.6,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkTrippletMassLower = B_lo-100,
        TrkTrippletMassUpper = B_hi+100,
        BMassLower = B_lo,
        BMassUpper = B_hi,
        Chi2Cut = 4.,
        JpsiContainerKey = "BPHY25OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))),
        TrackSelectorTool = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))),
        UseMassConstraint = True)
    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY25ThreeTrackReco_Bpm",
        VertexSearchTool = BPHY25Bpm_Jpsi1Trk,# Private tool handle
        OutputVtxContainerName = "BPHY25ThreeTrack_Bpm",
        PVContainerName = "PrimaryVertices",
        RelinkTracks = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if flags.Tracking.doLargeD0 else ["InDetTrackParticles"],
        V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))),
        PVRefitter = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))),
        RefitPV = True,
        MaxPVrefit = 500,
        RefPVContainerName = "BPHY25_BpmRefittedPrimaryVertices",
        DoVertexType = 7))
    return acc


def Cfg(flags):
    acc = ComponentAccumulator()
    return acc


def Cfg(flags):
    acc = ComponentAccumulator()
    return acc


def BPHY25Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (
        BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
        BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg,
        AugOriginalCountsCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()

    # Adds primary vertex counts and track counts to EventInfo before they are thinned
    acc.merge(
        AugOriginalCountsCfg(flags, name = "BPHY25_AugOriginalCounts"))

    doLRT = flags.Tracking.doLargeD0
    mainIDInput = "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles"
    if flags.Tracking.doLargeD0:
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg( flags, OutputTrackParticleLocation = "InDetWithLRTTrackParticles" if flags.Tracking.doLargeD0 else "InDetTrackParticles" ))

    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg
    from InDetConfig.InDetTrackSelectorToolConfig import V0InDetConversionTrackSelectorToolCfg
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

    # mass limits and constants used in the following
    Jpsi_lo = 2600.0
    Jpsi_hi = 3500.0
    B_lo = 5080.0
    B_hi = 5480.0
    Ks_lo = 430.0
    Ks_hi = 565.0
    Ld_lo = 1030.0
    Ld_hi = 1200.0
    Xi_lo = 1260.0
    Xi_hi = 1383.0
    Omg_lo = 1600.0
    Omg_hi = 1745.0
    Ldb0_lo = 5310.0
    Ldb0_hi = 5910.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Ksmass = 497.611
    Jpsimass = 3096.916
    B0mass = 5279.66
    Lambdamass = 1115.683
    Ximass = 1321.71
    Omegamass = 1672.45
    Lambdab0mass = 5619.60

    acc.merge(BPHY25_Reco_mumuCfg(flags))

    # B0 -> J/psi + K pi
    acc.merge(BPHY25_Reco_mumuCfg(flags))

    # B+ -> J/psi K
    acc.merge(BPHY25ThreeTrackReco_BpmCfg(flags))

    acc.addEventAlgo(CompFactory.DerivationFramework.Select_onia2mumu(
        name = "BPHY25Select_Jpsi",
        HypothesisName = "Jpsi",
        InputVtxContainerName = "BPHY25OniaCandidates",
        V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))),
        TrkMasses = [Mumass, Mumass],
        MassMin = Jpsi_lo,
        MassMax = Jpsi_hi,
        DoVertexType = 0))

    acc.addEventAlgo(CompFactory.DerivationFramework.Select_onia2mumu(
        name = "BPHY25Select_mumu",
        HypothesisName = "mumu",
        InputVtxContainerName = "BPHY25OniaCandidates",
        V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))),
        TrkMasses = [Mumass, Mumass],
        MassMin = 500.,
        MassMax = 4500.,
        DoVertexType = 0))

    Collections = [ ]
    RefPVContainers = [ ]
    RefPVAuxContainers = [ ]
    passedCandidates = [ ]

    #####################################################
    ## Xi_b^- -> J/psi Xi-, Xi^- -> Lambda pi-         ##
    ## Omega_b^- -> J/psi Omega-, Omega^- -> Lambda K- ##
    #####################################################

    list_disV_hypo = ["JpsiXi", "JpsiOmg", "mumuXi"]
    list_disV_jxHypo = ["Jpsi", "Jpsi", "mumu"]
    list_disV_jxCons = [True, True, False]
    list_disV_disVLo = [Xi_lo, Omg_lo, Xi_lo]
    list_disV_disVHi = [Xi_hi, Omg_hi, Xi_hi]
    list_disV_disVDau3Mass = [Pimass, Kmass, Pimass]
    list_disV_disVDaug3MinPt = [650., 750., 650.]
    list_disV_jxMass = [Jpsimass, Jpsimass, -9999.]
    list_disV_disVMass = [Ximass, Omegamass, Ximass]

    for i in range(len(list_disV_hypo)):
        name = "BPHY25_"+ list_disV_hypo[i]
        disVargs = { }
        disVargs.setdefault("JXVtxHypoNames", [list_disV_jxHypo[i]])
        if i == 0:
            # create V0 container for all following instances
            disVargs.setdefault("OutputV0VtxCollection", "V0Collection")
        else:
            disVargs.setdefault("V0Vertices", "V0Collection")
        disVargs.setdefault("DisplacedMassLowerCut", list_disV_disVLo[i])
        disVargs.setdefault("DisplacedMassUpperCut", list_disV_disVHi[i])
        disVargs.setdefault("CascadeVertexCollections", ["BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1","BPHY25_"+list_disV_hypo[i]+"_CascadeMainVtx"])
        disVargs.setdefault("HypothesisName", list_disV_hypo[i])
        disVargs.setdefault("DisVDaug3MassHypo", list_disV_disVDau3Mass[i])
        disVargs.setdefault("DisVDaug3MinP", list_disV_disVDaug3MinPt[i])
        disVargs.setdefault("JpsiMass", list_disV_jxMass[i])
        disVargs.setdefault("DisVtxMass", list_disV_disVMass[i])
        disVargs.setdefault("ApplyJpsiMassConstrain", list_disV_jxCons[i])
        acc.merge(BPHY25JpsiXPlusDisplaced_DisVCfg(flags, name = name, **disVargs))
        Collections += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    #############
    ## B+ + V0 ##
    #############
    list_BpmV0_hypo = ["BpmLd", "BpmKs"]
    list_BpmV0_jxInput = ["BPHY25ThreeTrack_Bpm", "BPHY25ThreeTrack_Bpm"]
    list_BpmV0_jxDau3Mass = [Kmass, Kmass]
    list_BpmV0_jxMassLo = [B_lo, B_lo]
    list_BpmV0_jxMassHi = [B_hi, B_hi]
    list_BpmV0_v0hypo = ["Lambda", "Ks"]

    list_BpmV0_obj = []

    for i in range(len(list_BpmV0_hypo)):
        name = "BPHY25_" + list_BpmV0_hypo[i]
        BpmV0args = { }
        BpmV0args.setdefault("JXVertices", list_BpmV0_jxInput[i])
        BpmV0args.setdefault("JXMassLowerCut", list_BpmV0_jxMassLo[i])
        BpmV0args.setdefault("JXMassUpperCut", list_BpmV0_jxMassHi[i])
        BpmV0args.setdefault("CascadeVertexCollections", ["BPHY25_"+list_BpmV0_hypo[i]+"_CascadeVtx1","BPHY25_"+list_BpmV0_hypo[i]+"_CascadeVtx2","BPHY25_"+list_BpmV0_hypo[i]+"_CascadeMainVtx"])
        BpmV0args.setdefault("V0Hypothesis", list_BpmV0_v0hypo[i])
        BpmV0args.setdefault("HypothesisName", list_BpmV0_hypo[i])
        BpmV0args.setdefault("JXDaug3MassHypo", list_BpmV0_jxDau3Mass[i])
        acc.merge(BPHY25JpsiXPlusDisplaced_BpmV0Cfg(flags, name, **BpmV0args))
        Collection += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    #################################
    ## B0(J/psi + K + pi) + Lambda ##
    #################################

    list_B0Ld_hypo = ["B0KpiLd", "B0piKLd"]
    list_B0Ld_jxInput = ["BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_B0Ld_jxMass = [B0mass, B0mass]
    list_B0Ld_jxDau3Mass = [Kmass, Pimass]
    list_B0Ld_jxDau4Mass = [Pimass, Kmass]
    list_B0Ld_jxMassLo = [B_lo, B_lo]
    list_B0Ld_jxMassHi = [B_hi, B_hi]

    for i in range(len(list_B0Ld_hypo)):
        name = "BPHY25_"+list_B0Ld_hypo[i]
        B0Ldargs = { }
        BOLdargs.setdefault("JXVertices", list_B0Ld_jxInput[i])
        BOLdargs.setdefault("JXMassLowerCut", list_B0Ld_jxMassLo[i])
        BOLdargs.setdefault("JXMassUpperCut", list_B0Ld_jxMassHi[i])
        BOLdargs.setdefault("CascadeVertexCollections", ["BPHY25_"+list_B0Ld_hypo[i]+"_CascadeVtx1","BPHY25_"+list_B0Ld_hypo[i]+"_CascadeVtx2","BPHY25_"+list_B0Ld_hypo[i]+"_CascadeMainVtx"])
        BOLdargs.setdefault("HypothesisName", list_B0Ld_hypo[i])
        BOLdargs.setdefault("JXDaug3MassHypo", list_B0Ld_jxDau3Mass[i])
        BOLdargs.setdefault("JXDaug4MassHypo", list_B0Ld_jxDau4Mass[i])
        BOLdargs.setdefault("JXMass", list_B0Ld_jxMass[i])
        acc.merge(BPHY25JpsiXPlusDisplaced_B0LdCfg(flags, name, **B0Ldargs))
        Collection += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    #############################
    ## B0(J/psi + K + pi) + Ks ##
    #############################

    list_B0Ks_hypo = ["B0KpiKs", "B0piKKs"]
    list_B0Ks_jxInput = ["BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_B0Ks_jxMass = [B0mass, B0mass]
    list_B0Ks_jxDau3Mass = [Kmass, Pimass]
    list_B0Ks_jxDau4Mass = [Pimass, Kmass]
    list_B0Ks_jxMassLo = [B_lo, B_lo]
    list_B0Ks_jxMassHi = [B_hi, B_hi]

    for i in range(len(list_B0Ks_hypo)):
        name = "BPHY25_"+list_B0Ks_hypo[i]
        B0Ksargs.setdefault("JXVertices", list_B0Ks_jxInput[i])
        B0Ksargs.setdefault("JXMassLowerCut", list_B0Ks_jxMassLo[i])
        B0Ksargs.setdefault("JXMassUpperCut", list_B0Ks_jxMassHi[i])
        B0Ksargs.setdefault("CascadeVertexCollections", ["BPHY25_"+list_B0Ks_hypo[i]+"_CascadeVtx1","BPHY25_"+list_B0Ks_hypo[i]+"_CascadeVtx2","BPHY25_"+list_B0Ks_hypo[i]+"_CascadeMainVtx"])
        B0Ksargs.setdefault("HypothesisName", list_B0Ks_hypo[i])
        B0Ksargs.setdefault("JXDaug3MassHypo", list_B0Ks_jxDau3Mass[i])
        B0Ksargs.setdefault("JXDaug4MassHypo", list_B0Ks_jxDau4Mass[i])
        B0Ksargs.setdefault("JXMass", list_B0Ks_jxMass[i])
        acc.merge(BPHY25JpsiXPlusDisplaced_B0KsCfg(flags, name, kwargs))
        Collection += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    #################
    ## J/psi + 2V0 ##
    #################

    list_2V0A_hypo = ["Jpsi2V0A"]

    for i in range(len(list_2V0A_hypo)):
        TwoV0Aargs = { }
        name = "BPHY25_"+list_2V0A_hypo[i]
        TwoV0Aargs.setdefault("CascadeVertexCollections", ["BPHY25_"+list_2V0A_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2V0A_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2V0A_hypo[i]+"_CascadeMainVtx"])
        TwoV0Aargs.setdefault("HypothesisName", list_2V0A_hypo[i])
        acc.merge(BPHY25JpsiXPlus2V0ACfg(flags, name, **TwoV0Aargs)
        Collection += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    list_2V0B_hypo        = ["Jpsi2V0B1", "Jpsi2V0B2"]
    list_2V0B_v02hypo     = ["Lambda", "Ks"]
    list_2V0B_jxv02mass   = [Lambdab0mass, B0mass]
    list_2V0B_jxv02massLo = [Ldb0_lo, B_lo]
    list_2V0B_jxv02massHi = [Ldb0_hi, B_hi]

    for i in range(len(list_2V0B_hypo)):
        TwoV0Bargs = { }
        name = "BPHY25_"+list_2V0B_hypo[i]
        TwoV0Bargs.setdefault("JXV02MassLowerCut", list_2V0B_jxv02massLo[i])
        TwoV0Bargs.setdefault("JXV02MassUpperCut", list_2V0B_jxv02massHi[i])
        TwoV0Bargs.setdefault("CascadeVertexCollections", ["BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx3","BPHY25_"+list_2V0B_hypo[i]+"_CascadeMainVtx"])
        TwoV0Bargs.setdefault("V02Hypothesis", list_2V0B_v02hypo[i])
        TwoV0Bargs.setdefault("HypothesisName", list_2V0B_hypo[i])
        TwoV0Bargs.setdefault("JXV02VtxMass", list_2V0B_jxv02mass[i])
        acc.merge(BPHY25JpsiXPlus2V0BCfg(flags, name, **TwoV0Bargs))
        Collection += acc.getEventAlgo(name).CascadeVertexCollections
        passedCandidates += ["BPHY25_" + acc.getEventAlgo(name).HypothesisName + "_CascadeMainVtx"]

    RefPVContainers += [
        "xAOD::VertexContainer#BPHY25_mumuRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_BpmRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_B0RefittedPrimaryVertices"
    ]
    RefPVAuxContainers += [
        "xAOD::VertexAuxContainer#BPHY25_mumuRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_BpmRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_B0RefittedPrimaryVerticesAux."
    ]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY25_SelectEvent)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY25Kernel",
        SkimmingTools     = [BPHY25_SelectEvent]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY25SlimmingHelper = SlimmingHelper("BPHY25SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY25_AllVariables  = getDefaultAllVariables()
    BPHY25_StaticContent = []

    # Needed for trigger objects
    BPHY25SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY25SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    BPHY25_AllVariables += ["PrimaryVertices"]
    BPHY25_StaticContent += RefPVContainers
    BPHY25_StaticContent += RefPVAuxContainers

    ## ID track particles
    BPHY25_AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"]

    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are stored in InDetTrackParticles collection)
    BPHY25_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    ## muon container
    BPHY25_AllVariables += ["Muons", "MuonSegments"]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for collection in Collections:
        BPHY25_StaticContent += ["xAOD::VertexContainer#%s" % collection]
        BPHY25_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % collection]

    # Truth information for MC only
    if flags.Input.isMC:
        BPHY25_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY25SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles", "InDetLargeD0TrackParticles"]
    BPHY25SlimmingHelper.AllVariables = BPHY25_AllVariables
    BPHY25SlimmingHelper.StaticContent = BPHY25_StaticContent

    BPHY25ItemList = BPHY25SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY25", ItemList=BPHY25ItemList, AcceptAlgs=["BPHY25Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY25", AcceptAlgs=["BPHY25Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
