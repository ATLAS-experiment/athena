# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the LLP tools used in DAOD_PHYSVAL
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PhysValLLPCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    TrackLocation = "InDetTrackParticles"
    MuonLocation = "Muons"
    ElectronLocation = "Electrons"

    if flags.Tracking.doLargeD0:
        # LRT track merge
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))
        TrackLocation = "InDetWithLRTTrackParticles"

        # LRT muons merge
        from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
        acc.merge(LRTMuonMergerAlg( flags,
                                    PromptMuonLocation    = "Muons",
                                    LRTMuonLocation       = "MuonsLRT",
                                    OutputMuonLocation    = "StdWithLRTMuons",
                                    CreateViewCollection  = True))
        MuonLocation = "StdWithLRTMuons"

        # LRT electrons merge
        from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
        acc.merge(LRTElectronMergerAlg( flags,
                                        PromptElectronLocation = "Electrons",
                                        LRTElectronLocation    = "LRTElectrons",
                                        OutputCollectionName   = "StdWithLRTElectrons",
                                        isDAOD                 = False,
                                        CreateViewCollection   = True))
        ElectronLocation = "StdWithLRTElectrons"

    # LLP Secondary Vertexing
    from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg

    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive",
                                 AugmentingVersionString  = "",
                                 FillIntermediateVertices = False,
                                 TrackLocation            = TrackLocation))

    # leptons-only VSI
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet_"+"_LeptonsMod_LRTR3_1p0",
                                 AugmentingVersionString     = "_LeptonsMod_LRTR3_1p0",
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = TrackLocation,
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectTracksWithLRTCuts   = True,
                                 doSelectTracksFromMuons     = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doSelectTracksFromElectrons = True,
                                 MuonLocation                = MuonLocation,
                                 ElectronLocation            = ElectronLocation))

    return acc
