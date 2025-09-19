# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_LLP1.py
# This defines DAOD_LLP1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority
# of physics analyses in ATLAS.
# It requires the flag LLP1 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory

MergedElectronContainer = "StdWithLRTElectrons"
MergedMuonContainer = "StdWithLRTMuons"
MergedMuonContainer_wZPH = "StdWithLRTMuons_wZPH"
MergedTrackCollection = "InDetWithLRTTrackParticles"
MergedTrackletCollection = "InDetDisappearingWithLRTTrackParticles"
MergedGSFTrackCollection = "InDetWithLRTGSFTrackParticles"
LLP1VrtSecInclusiveSuffixes = []
LLP1NewVSISuffixes = []

# Main algorithm config
def LLP1KernelCfg(flags, name='LLP1Kernel', **kwargs):

    """Configure the derivation framework driving algorithm (kernel) for LLP1"""
    acc = ComponentAccumulator()

    # Augmentations
    

    # LRT track merge
    from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
    acc.merge(InDetLRTMergeCfg(flags))
    acc.merge(InDetLRTMergeCfg(flags, name="GSFTrackMergerAlg", InputTrackParticleLocations = ["GSFTrackParticles", "LRTGSFTrackParticles"], OutputTrackParticleLocation = MergedGSFTrackCollection, OutputTrackParticleLocationCopy = MergedGSFTrackCollection))
    acc.merge(InDetLRTMergeCfg(flags, name="InDetDisappearingLRTMerge",InputTrackParticleLocations = ["InDetDisappearingTrackParticles", "InDetLargeD0TrackParticles"],OutputTrackParticleLocation = MergedTrackletCollection))    

    # LRT muons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    acc.merge(LRTMuonMergerAlg( flags,
                                PromptMuonLocation    = "Muons",
                                LRTMuonLocation       = "MuonsLRT",
                                OutputMuonLocation    = MergedMuonContainer,
                                CreateViewCollection  = True,
                                UseRun3WP = flags.GeoModel.Run == LHCPeriod.Run3))

    # LRT electrons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
    acc.merge(LRTElectronMergerAlg( flags,
                                    PromptElectronLocation = "Electrons",
                                    LRTElectronLocation    = "LRTElectrons",
                                    OutputCollectionName   = MergedElectronContainer,
                                    isDAOD                 = False,
                                    CreateViewCollection   = True))

    # Max Cell sum decoration tool
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        MaxCellDecoratorCfg, MaxCellDecoratorKernelCfg)

    # Default configuration
    acc.merge(MaxCellDecoratorKernelCfg(flags))

    # Specific for LRTElectrons
    LLP1LRTMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
        flags,
        name = "LLP1LRTMaxCellDecoratorTool",
        SGKey_electrons = "LRTElectrons",
        SGKey_egammaClusters = ("" if flags.GeoModel.Run == LHCPeriod.Run3
                                else "egammaClusters"),
        SGKey_photons = ''))
    acc.addPublicTool(LLP1LRTMaxCellDecoratorTool)

    # Vertex constraint tools
    from TrkConfig.TrkVertexFitterUtilsConfig import AtlasFullLinearizedTrackFactoryCfg
    AtlasFullLinearizedTrackFactoryTool = acc.popToolsAndMerge(AtlasFullLinearizedTrackFactoryCfg(flags,
                                                                                                  name = "LLP1AtlasFullLinearizedTrackFactory"))
    acc.addPublicTool(AtlasFullLinearizedTrackFactoryTool)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    ExtrapolatorTool = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags,
                                                                 name = "LLP1ExtrapolatorTool"))
    acc.addPublicTool(ExtrapolatorTool)


    from DerivationFrameworkLLP.LLPToolsConfig import TrackParametersKVUCfg
    LLP1TrackParametersKVUTool = acc.getPrimaryAndMerge(TrackParametersKVUCfg(flags,
                                                                              name                       = "LLP1TrackParametersKVU",
                                                                              TrackParticleContainerName = "InDetDisappearingTrackParticles",
                                                                              VertexContainerName        = "PrimaryVertices",
                                                                              LinearizedTrackFactory     = AtlasFullLinearizedTrackFactoryTool,
                                                                              TrackExtrapolator          = ExtrapolatorTool))
    acc.addPublicTool(LLP1TrackParametersKVUTool)

    # Track isolation tools
    import ROOT
    isoPar = ROOT.xAOD.Iso.IsolationType
    deco_ptcones = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20]
    deco_ptcones_suffix = ["ptcone40", "ptcone30", "ptcone20"]
    deco_prefix = 'LLP1_'

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
    TrackSelectionToolStd = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                   name = "TrackSelectionToolStd",
                                                                                   maxZ0SinTheta = 3.0,
                                                                                   minPt = 1000.))

    TrackSelectionToolPdEdx = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                     name = "TrackSelectionToolPdEdx",
                                                                                     maxD0 = 0.5,
                                                                                     maxZ0SinTheta = 3.0,
                                                                                     minPt = 1000.))

    TrackSelectionToolPdEdxTight = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                          name = "TrackSelectionToolPdEdxTight",
                                                                                          maxD0 = 0.5,
                                                                                          maxZ0SinTheta = 0.5,
                                                                                          minPt = 1000.))

    from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg, CaloIsolationToolCfg
    TrackIsoToolStd = acc.popToolsAndMerge(TrackIsolationToolCfg(flags,
                                                                 name = "TrackIsoToolStd",
                                                                 TrackSelectionTool = TrackSelectionToolStd))
    acc.addPublicTool(TrackIsoToolStd)

    TrackIsoToolPdEdx = acc.popToolsAndMerge(TrackIsolationToolCfg(flags,
                                                                   name = "TrackIsoToolPdEdx",
                                                                   TrackSelectionTool = TrackSelectionToolPdEdx))
    acc.addPublicTool(TrackIsoToolPdEdx)

    TrackIsoToolPdEdxTight = acc.popToolsAndMerge(TrackIsolationToolCfg(flags,
                                                                        name = "TrackIsoToolPdEdxTight",
                                                                        TrackSelectionTool = TrackSelectionToolPdEdxTight))
    acc.addPublicTool(TrackIsoToolPdEdxTight)

    from CaloIdentifier import SUBCALO
    CaloIsoTool = acc.popToolsAndMerge(CaloIsolationToolCfg(flags,
                                                            name = "CaloIsoTool",
                                                            EMCaloNums = [SUBCALO.LAREM],
                                                            HadCaloNums = [SUBCALO.LARHEC, SUBCALO.TILE],
                                                            UseEMScale  = True,
                                                            UseCaloExtensionCaching = False,
                                                            saveOnlyRequestedCorrections = True))
    acc.addPublicTool(CaloIsoTool)

    from DerivationFrameworkInDet.InDetToolsConfig import IsolationTrackDecoratorCfg
    LLP1IsolationTrackDecoratorTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                        name               = "LLP1IsolationTrackDecorator",
                                                                                        TrackIsolationTool = TrackIsoToolStd,
                                                                                        CaloIsolationTool  = CaloIsoTool,
                                                                                        TargetContainer    = "InDetTrackParticles",
                                                                                        SelectionString    = "InDetTrackParticles.pt>10*GeV",
                                                                                        iso                = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20, isoPar.ptvarcone40, isoPar.ptvarcone30, isoPar.ptvarcone20, isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20],
                                                                                        isoSuffix          = ["ptcone40", "ptcone30", "ptcone20", "ptvarcone40", "ptvarcone30", "ptvarcone20", "topoetcone40", "topoetcone30", "topoetcone20"],
                                                                                        Prefix             = deco_prefix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorTool)

    LLP1IsolationTrackDecoratorDTTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                          name               = "LLP1IsolationTrackDecoratorDT",
                                                                                          TrackIsolationTool = TrackIsoToolStd,
                                                                                          CaloIsolationTool  = CaloIsoTool,
                                                                                          TargetContainer    = "InDetDisappearingTrackParticles",
                                                                                          SelectionString    = "InDetDisappearingTrackParticles.pt>10*GeV",
                                                                                          iso                = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20, isoPar.ptvarcone40, isoPar.ptvarcone30, isoPar.ptvarcone20, isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20],
                                                                                          isoSuffix          = ["ptcone40", "ptcone30", "ptcone20", "ptvarcone40", "ptvarcone30", "ptvarcone20", "topoetcone40", "topoetcone30", "topoetcone20"],
                                                                                          Prefix             = deco_prefix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorDTTool)

    LLP1IsolationTrackDecoratorPdEdxTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                             name               = "LLP1IsolationTrackDecoratorPdEdx",
                                                                                             TrackIsolationTool = TrackIsoToolPdEdx,
                                                                                             CaloIsolationTool  = CaloIsoTool,
                                                                                             TargetContainer    = "InDetTrackParticles",
                                                                                             iso                = deco_ptcones,
                                                                                             Prefix             = 'TrkIsoPtPdEdx_',
                                                                                             isoSuffix          = deco_ptcones_suffix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorPdEdxTool)

    LLP1IsolationTrackDecoratorPdEdxDTTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                               name               = "LLP1IsolationTrackDecoratorPdEdxDT",
                                                                                               TrackIsolationTool = TrackIsoToolPdEdx,
                                                                                               CaloIsolationTool  = CaloIsoTool,
                                                                                               TargetContainer    = "InDetDisappearingTrackParticles",
                                                                                               iso                = deco_ptcones,
                                                                                               Prefix             = 'TrkIsoPtPdEdx_',
                                                                                               isoSuffix          = deco_ptcones_suffix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorPdEdxDTTool)

    LLP1IsolationTrackDecoratorPdEdxTightTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                                  name               = "LLP1IsolationTrackDecoratorPdEdxTight",
                                                                                                  TrackIsolationTool = TrackIsoToolPdEdxTight,
                                                                                                  CaloIsolationTool  = CaloIsoTool,
                                                                                                  TargetContainer    = "InDetTrackParticles",
                                                                                                  iso                = deco_ptcones,
                                                                                                  Prefix             = 'TrkIsoPtTightPdEdx_',
                                                                                                  isoSuffix          = deco_ptcones_suffix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorPdEdxTightTool)

    LLP1IsolationTrackDecoratorPdEdxTightDTTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                                    name               = "LLP1IsolationTrackDecoratorPdEdxTightDT",
                                                                                                    TrackIsolationTool = TrackIsoToolPdEdxTight,
                                                                                                    CaloIsolationTool  = CaloIsoTool,
                                                                                                    TargetContainer    = "InDetDisappearingTrackParticles",
                                                                                                    iso                = deco_ptcones,
                                                                                                    Prefix             = 'TrkIsoPtTightPdEdx_',
                                                                                                    isoSuffix          = deco_ptcones_suffix))
    acc.addPublicTool(LLP1IsolationTrackDecoratorPdEdxTightDTTool)

    from DerivationFrameworkLLP.LLPToolsConfig import TrackParticleCaloCellDecoratorCfg
    LLP1TrackParticleCaloCellDecoratorTool = acc.getPrimaryAndMerge(TrackParticleCaloCellDecoratorCfg(flags,
                                                                                                      name               = "LLP1TrackParticleCaloCellDecorator",
                                                                                                      DecorationPrefix   = "LLP1",
                                                                                                      ContainerName      = "InDetTrackParticles"))
    acc.addPublicTool(LLP1TrackParticleCaloCellDecoratorTool)

    augmentationTools = [ LLP1LRTMaxCellDecoratorTool,
                          LLP1TrackParametersKVUTool,
                          LLP1IsolationTrackDecoratorTool,
                          LLP1IsolationTrackDecoratorDTTool,
                          LLP1IsolationTrackDecoratorPdEdxTool,
                          LLP1IsolationTrackDecoratorPdEdxDTTool,
                          LLP1IsolationTrackDecoratorPdEdxTightTool,
                          LLP1IsolationTrackDecoratorPdEdxTightDTTool,
                          LLP1TrackParticleCaloCellDecoratorTool ]

    # Specific for Taus
    LLP1TauMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
        flags,
        name = "LLP1TauMaxCellDecoratorTool",
        SGKey_taus = 'TauJets',
        SGKey_electrons = '',
        SGKey_photons = ''))
    acc.addPublicTool(LLP1TauMaxCellDecoratorTool)

    augmentationTools += [ LLP1TauMaxCellDecoratorTool ]

    # Specific for Jets: AntiKt4EMTopoJets
    LLP1AntiKt4EMTopoJetMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
        flags,
        name = "LLP1AntiKt4EMTopoJetMaxCellDecoratorTool",
        SGKey_jets = 'AntiKt4EMTopoJets',
        SGKey_taus = '',
        SGKey_electrons = '',
        SGKey_photons = ''))
    acc.addPublicTool(LLP1AntiKt4EMTopoJetMaxCellDecoratorTool)

    augmentationTools += [ LLP1AntiKt4EMTopoJetMaxCellDecoratorTool ]

    # Specific for Jets: AntiKt4EMPFlowJets
    LLP1AntiKt4EMPFlowJetMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
        flags,
        name = "LLP1AntiKt4EMPFlowJetMaxCellDecoratorTool",
        SGKey_jets = 'AntiKt4EMPFlowJets',
        SGKey_taus = '',
        SGKey_electrons = '',
        SGKey_photons = ''))
    acc.addPublicTool(LLP1AntiKt4EMPFlowJetMaxCellDecoratorTool)

    augmentationTools += [ LLP1AntiKt4EMPFlowJetMaxCellDecoratorTool ]
                                           
    # Reclustered jets definitions
    from JetRecConfig.JetRecConfig import registerAsInputConstit, JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4Truth, AntiKt4EMTopo
    from JetRecConfig.JetDefinition import JetDefinition
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst

    registerAsInputConstit(AntiKt4EMTopo)
    registerAsInputConstit(AntiKt4Truth)
    cst.AntiKt4EMTopoJets.label = "EMTopoRC"
    cst.AntiKt4TruthJets.label = "TruthRC"

    AntiKt10RCEMTopo = JetDefinition(   "AntiKt",1.0,cst.AntiKt4EMTopoJets,
                                        ghostdefs = ["Track", "TrackLRT", "LCTopoOrigin"],
                                        modifiers = ("Sort", "Filter:200000",),
                                        standardRecoMode = True,
                                        lock = True,
    )
    if flags.Input.isMC:
        AntiKt10RCTruth = JetDefinition("AntiKt",1.0,cst.AntiKt4TruthJets,
                                        ghostdefs = [],
                                        modifiers = ("Sort", "Filter:200000",),
                                        standardRecoMode = True,
                                        lock = True
        )

    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    acc.merge(JetRecCfg(flags,AntiKt10RCEMTopo))
    if flags.Input.isMC: acc.merge(JetRecCfg(flags,AntiKt10RCTruth))

    # MET with LRT in association map
    from DerivationFrameworkJetEtMiss.METCommonConfig import METLRTCfg
    acc.merge(METLRTCfg(flags, "AntiKt4EMTopo"))
    acc.merge(METLRTCfg(flags, "AntiKt4EMPFlow"))

    # LRT Egamma
    from DerivationFrameworkEGamma.EGammaLRTConfig import EGammaLRTCfg
    acc.merge(EGammaLRTCfg(flags))

    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronLHSelectorsCfg
    acc.merge(LRTElectronLHSelectorsCfg(flags))

    #Photon ID Selector
    from DerivationFrameworkLLP.LLPToolsConfig import PhotonIsEMSelectorsCfg
    acc.merge(PhotonIsEMSelectorsCfg(flags))

    # LRT Muons
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(flags,
                             suff="LRT"))

    # Recover Zero Pixel Hit Muons
    from DerivationFrameworkLLP.LLPToolsConfig import RecoverZeroPixelHitMuonsCfg
    acc.merge(RecoverZeroPixelHitMuonsCfg(flags))
    
    # flavor tagging
    from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
    acc.merge(FlavorTaggingCfg(flags, 'AntiKt4EMTopoJets'))

    # VrtSecInclusive
    from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg

    # MuSAVtxFitter
    from MuSAVtxFitter.MuSAVtxFitterConfig import MuSAVtxFitterConfig, MuSAVtxJPsiValidationAlgCfg, MuSAVtxFitterValidationConfig

    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive",
                                 AugmentingVersionString  = "",
                                 FillIntermediateVertices = False,
                                 TrackLocation            = MergedTrackCollection))
    LLP1VrtSecInclusiveSuffixes.append("")

    # short-lifetime VSI
    shortLifetimeSuffix = "_shortLifetime"
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet"+shortLifetimeSuffix,
                                 AugmentingVersionString     = shortLifetimeSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 twoTrkVtxFormingD0Cut       = 1.0))
    LLP1VrtSecInclusiveSuffixes.append(shortLifetimeSuffix)

    # disappearing track + LRT VSI 
    dissapearingSuffix = "_disappearing"
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_"+dissapearingSuffix,
                                 AugmentingVersionString     = dissapearingSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackletCollection,
                                 doReassembleVertices        = True,
                                 doMergeByShuffling          = False,
                                 doMergeFinalVerticesDistance= False,
                                 doAssociateNonSelectedTracks= False,
                                 DoPVcompatibility           = True,
                                 RemoveFake2TrkVrt           = False,
                                 PassThroughTrackSelection   = True,
                                 TruncateListOfWorkingVertices = False,
                                 twoTrkVtxFormingD0Cut       = 0.0,
                                 SelVrtChi2Cut               = 1000000.0,
                                 twoTrVrtMaxPerigeeDist      = 50.0,
                                 twoTrVrtMinRadius           = 50.0,
                                 doDisappearingTrackVertexing= True
    ))
    LLP1VrtSecInclusiveSuffixes.append(dissapearingSuffix)    


    
    if flags.Input.isMC and flags.Derivation.LLP.doTrackSystematics:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import TrackSystematicsAlgCfg
        TrackSystSuffix = "_TRK_EFF_LARGED0_GLOBAL__1down"
        acc.merge(TrackSystematicsAlgCfg(
            flags,
            name=f"InDetTrackSystematicsAlg{TrackSystSuffix}",
            InputTrackContainer  = MergedTrackCollection,
            OutputTrackContainer = f"{MergedTrackCollection}{TrackSystSuffix}"))
        acc.merge(VrtSecInclusiveCfg(flags,
                                     name = f"VrtSecInclusive{TrackSystSuffix}",
                                     AugmentingVersionString  = TrackSystSuffix,
                                     FillIntermediateVertices = False,
                                     TrackLocation            = f"{MergedTrackCollection}{TrackSystSuffix}"))
        LLP1VrtSecInclusiveSuffixes.append(TrackSystSuffix)
    
        TrackSystSuffixShortLifetime = "_TRK_EFF_LARGED0_GLOBAL__1down_shortLifetime"
        acc.merge(TrackSystematicsAlgCfg(
            flags,
            name=f"InDetTrackSystematicsAlg{TrackSystSuffixShortLifetime}",
            InputTrackContainer  = MergedTrackCollection,
            OutputTrackContainer = f"{MergedTrackCollection}{TrackSystSuffixShortLifetime}"))
        acc.merge(VrtSecInclusiveCfg(flags,
                                     name = f"VrtSecInclusive{TrackSystSuffixShortLifetime}",
                                     AugmentingVersionString  = TrackSystSuffixShortLifetime,
                                     FillIntermediateVertices = False,
                                     TrackLocation            = f"{MergedTrackCollection}{TrackSystSuffixShortLifetime}",
                                     twoTrkVtxFormingD0Cut    = 1.0))
        LLP1VrtSecInclusiveSuffixes.append(TrackSystSuffixShortLifetime)

    # LRT muons merge
    from DerivationFrameworkLLP.LLPToolsConfig import ZeroPixelHitMuonMergerAlgCfg
    acc.merge(ZeroPixelHitMuonMergerAlgCfg(flags,
                                           InputMuonContainers  = [MergedMuonContainer, "ZeroPixelHitMuons"],
                                           OutputMuonLocation   = MergedMuonContainer_wZPH))


    # leptons-only VSI
    LeptonsSuffix = "_Leptons"
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet"+LeptonsSuffix,
                                 AugmentingVersionString     = LeptonsSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectTracksFromMuons     = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doSelectTracksFromElectrons = True,
                                 MuonLocation                = MergedMuonContainer,
                                 ElectronLocation            = MergedElectronContainer))
    LLP1VrtSecInclusiveSuffixes.append(LeptonsSuffix)

    # track VSI
    LepTrackSuffix = "_LepTrack"
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet"+LepTrackSuffix,
                                 AugmentingVersionString     = LepTrackSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 MuonLocation                = MergedMuonContainer,
                                 ElectronLocation            = MergedElectronContainer,
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectIDAndGSFTracks      = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doRemoveNonLeptonVertices   = True,
                                 doAssociateNonSelectedTracks= False))
    LLP1VrtSecInclusiveSuffixes.append(LepTrackSuffix)

    # Small-d0 Muons VSI
    BoostedMuonsSuffix = "_BoostedMuons"
    acc.merge(VrtSecInclusiveCfg(flags,
                                 name = "VrtSecInclusive_InDet"+BoostedMuonsSuffix,
                                 AugmentingVersionString     = BoostedMuonsSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 twoTrkVtxFormingD0Cut       = 0.0,
                                 doSelectTracksFromMuons     = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doSelectTracksFromElectrons = False,
                                 MuonLocation                = MergedMuonContainer_wZPH,
                                 do_PVvetoCut                = False,
                                 DoTwoTrSoftBtag             = True,
                                 TwoTrVrtMinDistFromPVCut    = 0.5,
                                 associatePtCut              = 500.))
    LLP1VrtSecInclusiveSuffixes.append(BoostedMuonsSuffix)

    # MuSA Vertices
    acc.merge(MuSAVtxFitterConfig(flags, 
                                      MuonContainerName=MergedMuonContainer))

    if flags.Derivation.LLP.doMuSAValidation:
        #Create JPsi tagged muon container
        acc.merge(MuSAVtxJPsiValidationAlgCfg(flags, 
                                           MuonContainer=MergedMuonContainer,
                                           JPsiMuonContainer="JPsiMuons"))
        
        # JPsi validation MuSA Vertices
        acc.merge(MuSAVtxFitterValidationConfig(flags,
                                                name="MuSAVtxFitterValidationJPsi",
                                                MuonContainerName="JPsiMuons"))
        # all MSTPs validation MuSA Vertices
        acc.merge(MuSAVtxFitterValidationConfig(flags,
                                                MuonContainerName=MergedMuonContainer,
                                                MuSAVtxContainerName="ValidationMuSAVertices",
                                                MuSAExtrapolatedTracksName="ValidationMuSAExtrapolatedTrackParticles",
                                                MSTPContainerName="MuonSpectrometerTrackParticles"))
                                            

    # NewVSI: LepTrack variation
    from NewVrtSecInclusiveTool.NewVrtSecInclusiveAlgConfig import NewVrtSecInclusiveAlgLLPCfg
    from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import DVFinderToolCfg
    IDAndGSFSuffix = "_IDAndGSF_LepTrack"

    NVSILepTrack_Tool = acc.popToolsAndMerge(DVFinderToolCfg(flags,FillHist=False,AugmentingVersionString=IDAndGSFSuffix,MaxZVrt=1000.,AntiPileupSigRCut=2.))                                
    acc.merge(NewVrtSecInclusiveAlgLLPCfg(flags, 
                                       algname = "NVSI"+IDAndGSFSuffix,
                                       AugmentingVersionString = IDAndGSFSuffix,
                                       ElectronContainer = MergedElectronContainer,
                                       MuonContainer = MergedMuonContainer,
                                       TrackParticleContainer = MergedTrackCollection,
                                       GSFTrackParticleContainer = MergedGSFTrackCollection,
                                       BVertexContainerName = "NewVrtSecInclusive_SecondaryVertices"+IDAndGSFSuffix,
                                       AddIDTracks = True,
                                       AddGSFTracks = True,
                                       RemoveNonLepVertices = True,
                                       BVertexTool = NVSILepTrack_Tool))
    LLP1NewVSISuffixes.append(IDAndGSFSuffix)
    
    # bad jet cleaning
    jet_clean_prefix="DFCommonJets_"
    jet_clean_container="AntiKt4EMTopoJets"
    jet_clean_level="SuperLooseBadLLP"
    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg, JetCleaningToolCfg
    LLP1JetCleanSuperLLPTool = acc.popToolsAndMerge(JetCleaningToolCfg(flags,
                                                                       "LLP1JetCleanSuperLLP",
                                                                       jet_clean_container,
                                                                       jet_clean_level,
                                                                       False))
    acc.addPublicTool(LLP1JetCleanSuperLLPTool)

    LLP1EventCleanSuperLLPTool = acc.popToolsAndMerge(EventCleaningToolCfg(flags,
                                                                           "LLP1EventCleanSuperLLP",
                                                                           jet_clean_level))
    LLP1EventCleanSuperLLPTool.JetCleanPrefix = jet_clean_prefix
    LLP1EventCleanSuperLLPTool.OrDecorator = "passOR_EMTopo"
    LLP1EventCleanSuperLLPTool.JetContainer = jet_clean_container
    LLP1EventCleanSuperLLPTool.JetCleaningTool = LLP1JetCleanSuperLLPTool
    acc.addPublicTool(LLP1EventCleanSuperLLPTool)

    LLP1EventCleanAlg = CompFactory.EventCleaningTestAlg(
        "LLP1JetCleanDecoratorSuperLLP",
        EventCleaningTool = LLP1EventCleanSuperLLPTool,
        JetCollectionName = jet_clean_container,
        EventCleanPrefix  = jet_clean_prefix,
        CleaningLevel     = jet_clean_level,
        doEvent           = True)

    # Sequence for decorator locking.
    # See comments in JetCommonConfig.AddEventCleanFlagsCfg.
    acc.addSequence(CompFactory.AthSequencer('EventCleanSeq', Sequential=True))
    acc.addEventAlgo(LLP1EventCleanAlg, 'EventCleanSeq')



    from DerivationFrameworkLLP.LLPToolsConfig import AugmentationToolLeadingJetsCfg
    augmentationToolLeadingJets = acc.getPrimaryAndMerge(AugmentationToolLeadingJetsCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, AugmentationTools = [augmentationToolLeadingJets]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, EgammaTrackParticleThinningCfg, MuonTrackParticleThinningCfg, TauTrackParticleThinningCfg, DiTauTrackParticleThinningCfg 
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
    from DerivationFrameworkTau.TauCommonConfig import TauThinningCfg

    # Inner detector tracks need to have greater than 10 GeV of pT
    LLP1TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "LLP1TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "InDetTrackParticles.pt>10*GeV",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    # Keep all GSF LRT Tracks 
    LLP1LRTGSFTrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "LLP1LRTGSFTrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "LRTGSFTrackParticles.pt>0*GeV",
        InDetTrackParticlesKey  = "LRTGSFTrackParticles"))
    # Pixel tracklets need to have greater than 5 GeV of pT
    LLP1DTTrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "LLP1DTTrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "InDetDisappearingTrackParticles.pt>5*GeV",
        InDetTrackParticlesKey  = "InDetDisappearingTrackParticles"))

    # Include inner detector tracks associated with electrons
    LLP1ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "LLP1ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    # Include inner detector tracks associated with LRT electrons
    LLP1LRTElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "LLP1LRTElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "LRTElectrons",
        InDetTrackParticlesKey  = "InDetLargeD0TrackParticles",
        GSFTrackParticlesKey    = "LRTGSFTrackParticles")) 
    # Include inner detector tracks associated with muons
    LLP1MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "LLP1MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    # Include LRT inner detector tracks associated with LRT muons
    LLP1LRTMuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "LLP1LRTMuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "MuonsLRT",
        InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))
    # GSF tracks associated to photons
    LLP1PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "LLP1PhotonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Photons",
        InDetTrackParticlesKey  = "",
        GSFConversionVerticesKey = "GSFConversionVertices",
        GSFTrackParticlesKey    = "GSFTrackParticles",
        BestMatchOnly            = True,
        BestVtxMatchOnly         = True)) 
    # Tau-related containers: taus, tau tracks and associated ID tracks, neutral PFOs, secondary vertices
    tau_thinning_expression = f"TauJets.pt >= {flags.Tau.MinPtDAOD}"
    LLP1TauJetsThinningTool = acc.getPrimaryAndMerge(TauThinningCfg(
        flags,
        name                 = "LLP1TauJetThinningTool",
        StreamName           = kwargs['StreamName'],
        Taus                 = "TauJets",
        TauTracks            = "TauTracks",
        TrackParticles       = "InDetTrackParticles",
        TauNeutralPFOs       = "TauNeutralParticleFlowObjects",
        TauSecondaryVertices = "TauSecondaryVertices",
        SelectionString      = tau_thinning_expression))

    # Only keep tau tracks (and associated ID tracks) classified as charged tracks
    LLP1TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "LLP1TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    tau_murm_thinning_expression = tau_thinning_expression.replace('TauJets', 'TauJets_MuonRM')
    LLP1TauJetMuonRMParticleThinningTool = acc.getPrimaryAndMerge(TauThinningCfg(
        flags,
        name                   = "LLP1TauJets_MuonRMThinningTool",
        StreamName             = kwargs['StreamName'],
        Taus                   = "TauJets_MuonRM",
        TauTracks              = "TauTracks_MuonRM",
        TrackParticles         = "InDetTrackParticles",
        TauNeutralPFOs         = "TauNeutralParticleFlowObjects_MuonRM",
        TauSecondaryVertices   = "TauSecondaryVertices_MuonRM",
        SelectionString        = tau_murm_thinning_expression))

    # ID tracks associated with high-pt di-tau
    LLP1DiTauTPThinningTool = acc.getPrimaryAndMerge(DiTauTrackParticleThinningCfg(
        flags,
        name                    = "LLP1DiTauTPThinningTool",
        StreamName              = kwargs['StreamName'],
        DiTauKey                = "DiTauJets",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    ## Low-pt di-tau thinning
    LLP1DiTauLowPtThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(
        flags,
        name            = "LLP1DiTauLowPtThinningTool",
        StreamName      = kwargs['StreamName'],
        ContainerName   = "DiTauJetsLowPt",
        SelectionString = "DiTauJetsLowPt.nSubjets > 1"))

    # ID tracks associated with low-pt ditau
    LLP1DiTauLowPtTPThinningTool = acc.getPrimaryAndMerge(DiTauTrackParticleThinningCfg(
        flags,
        name                    = "LLP1DiTauLowPtTPThinningTool",
        StreamName              = kwargs['StreamName'],
        DiTauKey                = "DiTauJetsLowPt",
        InDetTrackParticlesKey  = "InDetTrackParticles",
        SelectionString         = "DiTauJetsLowPt.nSubjets > 1"))


    # ID Tracks associated with secondary vertices
    from DerivationFrameworkLLP.LLPToolsConfig import VSITrackParticleThinningCfg
    LLP1VSITPThinningTool = acc.getPrimaryAndMerge(VSITrackParticleThinningCfg(flags,
                                                                               name                    = "LLP1VSITPThinningTool",
                                                                               StreamName              = kwargs['StreamName'],
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                               AugVerStrings = LLP1VrtSecInclusiveSuffixes + LLP1NewVSISuffixes))
    LLP1LRTVSITPThinningTool = acc.getPrimaryAndMerge(VSITrackParticleThinningCfg(flags,
                                                                                  name                    = "LLP1LRTVSITPThinningTool",
                                                                                  StreamName              = kwargs['StreamName'],
                                                                                  InDetTrackParticlesKey  = "InDetLargeD0TrackParticles",
                                                                                  AugVerStrings = LLP1VrtSecInclusiveSuffixes + LLP1NewVSISuffixes))
    LLP1GSFVSITPThinningTool = acc.getPrimaryAndMerge(VSITrackParticleThinningCfg(flags,
                                                                               name                    = "LLP1GSFVSITPThinningTool",
                                                                               StreamName              = kwargs['StreamName'],
                                                                               InDetTrackParticlesKey  = "GSFTrackParticles",
                                                                               AugVerStrings = [IDAndGSFSuffix]))

    # ID Tracks associated with jets
    from DerivationFrameworkLLP.LLPToolsConfig import JetTrackParticleThinningCfg, JetLargeD0TrackParticleThinningCfg
    LLP1JetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
                                                                               name                    = "LLP1JetTPThinningTool",
                                                                               StreamName              = kwargs['StreamName'],
                                                                               JetKey                  = "AntiKt4EMTopoJets",
                                                                               SelectionString         = "(AntiKt4EMTopoJets.pt > 20.*GeV) && (abs(AntiKt4EMTopoJets.eta) < 2.5)",
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles"))

    LLP1FatJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(  flags,
                                                                                    name                    = "LLP1FatJetTPThinningTool",
                                                                                    StreamName              = kwargs['StreamName'],
                                                                                    JetKey                  = "AntiKt10EMTopoRCJets",
                                                                                    SelectionString         = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                    InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                    ))

    # LRT Tracks associated with jets
    if flags.Tracking.doLargeD0:
        LLP1LRTJetTPThinningTool = acc.getPrimaryAndMerge(JetLargeD0TrackParticleThinningCfg(flags,
                                                                                             name                    = "LLP1LRTJetTPThinningTool",
                                                                                             StreamName              = kwargs['StreamName'],
                                                                                             JetKey                  = "AntiKt4EMTopoJets",
                                                                                             SelectionString         = "(AntiKt4EMTopoJets.pt > 20.*GeV) && (abs(AntiKt4EMTopoJets.eta) < 2.5)",
                                                                                             InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))

        LLP1LRTFatJetTPThinningTool = acc.getPrimaryAndMerge(JetLargeD0TrackParticleThinningCfg(flags,
                                                                                                name                    = "LLP1LRTFatJetTPThinningTool",
                                                                                                StreamName              = kwargs['StreamName'],
                                                                                                JetKey                  = "AntiKt10EMTopoRCJets",
                                                                                                SelectionString         = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                                InDetTrackParticlesKey  = "InDetLargeD0TrackParticles",
                                                                                                ))

    # high dE/dx and low pT tracks
    from DerivationFrameworkLLP.LLPToolsConfig import PixeldEdxTrackParticleThinningCfg
    LLP1PixeldEdxTrackParticleThinningTool = acc.getPrimaryAndMerge(PixeldEdxTrackParticleThinningCfg(
        flags,
        name                    = "LLP1PixeldEdxTrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        InDetTrackParticlesKey  = "InDetTrackParticles"))


    #Thinning CaloCalTopoClusters associated to AntiKt4EMTopoJets
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import JetCaloClusterThinningCfg

    LLP1CCThinningTool = acc.getPrimaryAndMerge(JetCaloClusterThinningCfg(
                                                                     flags,
                                                                     name                    = "LLP1CCTool",
                                                                     StreamName            = kwargs['StreamName'],
                                                                     SGKey                   = "AntiKt4EMTopoJets",
                                                                     TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                                     SelectionString         = "(AntiKt4EMTopoJets.DFDecoratorLeadingJets)",
                                                                     AdditionalClustersKey = ["EMOriginTopoClusters","LCOriginTopoClusters"] 
                                                                     ))
                                                                     
   
   


    # Finally the kernel itself
    thinningTools = [LLP1TrackParticleThinningTool,
                     LLP1DTTrackParticleThinningTool,
                     LLP1ElectronTPThinningTool,
                     LLP1LRTElectronTPThinningTool,
                     LLP1MuonTPThinningTool,
                     LLP1PhotonTPThinningTool,
                     LLP1LRTMuonTPThinningTool,
                     LLP1TauJetsThinningTool,
                     LLP1TauTPThinningTool,
                     LLP1TauJetMuonRMParticleThinningTool,
                     LLP1DiTauTPThinningTool,
                     LLP1DiTauLowPtThinningTool,
                     LLP1DiTauLowPtTPThinningTool,
                     LLP1VSITPThinningTool,
                     LLP1LRTVSITPThinningTool,
                     LLP1GSFVSITPThinningTool,
                     LLP1JetTPThinningTool,
                     LLP1FatJetTPThinningTool,
                     LLP1PixeldEdxTrackParticleThinningTool,
                     LLP1CCThinningTool,
                     LLP1LRTGSFTrackParticleThinningTool
                     ]

    if flags.Tracking.doLargeD0:
        thinningTools.append(LLP1LRTJetTPThinningTool)
        thinningTools.append(LLP1LRTFatJetTPThinningTool)

    # Additionnal augmentations

    # Compute RC substructure variables from energy clusters
    from DerivationFrameworkLLP.LLPToolsConfig import RCJetSubstructureAugCfg
    LLP1RCJetSubstructureClustTrimAugTool = acc.getPrimaryAndMerge(RCJetSubstructureAugCfg(flags,
                                                                                    name                              = "LLP1RCJetSubstructureClustTrimAugTool",
                                                                                    StreamName                        = kwargs['StreamName'],
                                                                                    JetContainerKey                   = "AntiKt10EMTopoRCJets",
                                                                                    SelectionString                   = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                    GhostConstitNames                 = ["GhostLCTopoOrigin"],
                                                                                    Suffix                            = "clusterTrim",
                                                                                    Grooming                          = "Trimming",
                                                                                    RClusTrim                         = 0.2,
                                                                                    PtFracTrim                        = 0.05
                                                                                    ))
    RCSubstructureClusterTrimAug = CompFactory.DerivationFramework.CommonAugmentation("RCSubstructureClusterTrimAug", AugmentationTools = [LLP1RCJetSubstructureClustTrimAugTool])
    acc.addEventAlgo(RCSubstructureClusterTrimAug)

    LLP1RCJetSubstructureClustSDAugTool = acc.getPrimaryAndMerge(RCJetSubstructureAugCfg(flags,
                                                                                    name                              = "LLP1RCJetSubstructureClustSDAugTool",
                                                                                    StreamName                        = kwargs['StreamName'],
                                                                                    JetContainerKey                   = "AntiKt10EMTopoRCJets",
                                                                                    SelectionString                   = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                    GhostConstitNames                 = ["GhostLCTopoOrigin"],
                                                                                    Suffix                            = "clusterSoftDrop",
                                                                                    Grooming                          = "SoftDrop",
                                                                                    BetaSoft                          = 1.0,
                                                                                    ZcutSoft                          = 0.1
                                                                                    ))
    RCSubstructureClusterSDAug = CompFactory.DerivationFramework.CommonAugmentation("RCSubstructureClusterSDAug", AugmentationTools = [LLP1RCJetSubstructureClustSDAugTool])
    acc.addEventAlgo(RCSubstructureClusterSDAug)

    # Compute RC substructure variables from tracks
    from DerivationFrameworkLLP.LLPToolsConfig import RCJetSubstructureAugCfg
    LLP1RCJetSubstructureTrackTrimAugTool = acc.getPrimaryAndMerge(RCJetSubstructureAugCfg( flags,
                                                                                        name                              = "LLP1RCJetSubstructureTrackTrimAugTool",
                                                                                        StreamName                        = kwargs['StreamName'],
                                                                                        JetContainerKey                   = "AntiKt10EMTopoRCJets",
                                                                                        SelectionString                   = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                        GhostConstitNames                 = ["GhostTrack", "GhostTrackLRT"],
                                                                                        Suffix                            = "trackTrim",
                                                                                        Grooming                          = "Trimming",
                                                                                        RClusTrim                         = 0.2,
                                                                                        PtFracTrim                        = 0.05
                                                                                        ))
    RCSubstructureTrackTrimAug = CompFactory.DerivationFramework.CommonAugmentation("RCSubstructureTrackTrimAug", AugmentationTools = [LLP1RCJetSubstructureTrackTrimAugTool])
    acc.addEventAlgo(RCSubstructureTrackTrimAug)

    from DerivationFrameworkLLP.LLPToolsConfig import RCJetSubstructureAugCfg
    LLP1RCJetSubstructureTrackSDAugTool = acc.getPrimaryAndMerge(RCJetSubstructureAugCfg( flags,
                                                                                        name                              = "LLP1RCJetSubstructureTrackSDAugTool",
                                                                                        StreamName                        = kwargs['StreamName'],
                                                                                        JetContainerKey                   = "AntiKt10EMTopoRCJets",
                                                                                        SelectionString                   = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                        GhostConstitNames                 = ["GhostTrack", "GhostTrackLRT"],
                                                                                        Suffix                            = "trackSoftDrop",
                                                                                        Grooming                          = "SoftDrop",
                                                                                        BetaSoft                          = 1.0,
                                                                                        ZcutSoft                          = 0.1
                                                                                        ))
    RCSubstructureTrackSDAug = CompFactory.DerivationFramework.CommonAugmentation("RCSubstructureTrackSDAug", AugmentationTools = [LLP1RCJetSubstructureTrackSDAugTool])
    acc.addEventAlgo(RCSubstructureTrackSDAug)



    # Skimming
    skimmingTools = []

    from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerSkimmingToolCfg
    LLP1TriggerSkimmingTool = acc.getPrimaryAndMerge(LLP1TriggerSkimmingToolCfg(flags,
                                                                                name = "LLP1TriggerSkimmingTool",
                                                                                TriggerListsHelper = kwargs['TriggerListsHelper']))

    skimmingTools.append(LLP1TriggerSkimmingTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = skimmingTools,
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))

    return acc






def LLP1Cfg(flags):
    acc = ComponentAccumulator()
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    LLP1TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(LLP1KernelCfg(flags, name="LLP1Kernel", StreamName = 'StreamDAOD_LLP1', TriggerListsHelper = LLP1TriggerListsHelper))

    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    ## Must set useSelTools to set elLHVLoose and phIsEMLoose with tools - not already set in LLP1 derivation
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
    # We can't pass non-LRT containers to the LLP algorithm.
    # Otherwise, if this is run in conjunction with PHYS, the decorations
    # produced by the PHYS algorithm will be overwritten.  To get those
    # decorations produced when LLP1 is run alone, we schedule a separate
    # algorithm for those but make it the same as the one in PHYS so that
    # they'll be merged when both formats are used together.
    acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, stream_name = 'StreamDAOD_LLP1'))
    contNames = [ "LRTElectrons", "MuonsLRT" ] 
    acc.merge(IsoCloseByAlgsCfg(flags, suff = "_LLP1", isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = 'StreamDAOD_LLP1', hasLRT = True))
    contNames = [ MergedMuonContainer, MergedElectronContainer, "Photons" ] 
    acc.merge(IsoCloseByAlgsCfg(flags, suff = "_LLP1_LRTMerged", isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = 'StreamDAOD_LLP1', isoDecSuffix = "CloseByCorr_LRT", caloDecSuffix = '_LRT', hasLRT = True))
    contNames = [ "ZeroPixelHitMuons" ]
    acc.merge(IsoCloseByAlgsCfg(flags, suff = "_LLP1_ZeroPixelHitsMuons", isPhysLite = False, containerNames = contNames, stream_name = 'StreamDAOD_LLP1', isoDecSuffix = "CloseByCorr_ZPH"))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    LLP1SlimmingHelper = SlimmingHelper("LLP1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    LLP1SlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "LRTElectrons",
                                           "Photons",
                                           "Muons",
                                           "MuonsLRT",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "InDetLargeD0TrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "AntiKt4EMPFlowJets_FTAG",
                                           "BTagging_AntiKt4EMTopo",
                                           "BTagging_AntiKt4EMPFlow",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]

    LLP1SlimmingHelper.AllVariables =  ["InDetDisappearingTrackParticles",
                                        "MSDisplacedVertex",
                                        "MuonSpectrometerTrackParticles",
                                        "UnAssocMuonSegments",
                                        "MuonSegments",
                                        "MuonSegments_LRT",
                                        "MSonlyTracklets",
                                        "CombinedMuonTrackParticles",
                                        "ExtrapolatedMuonTrackParticles",
                                        "CombinedMuonsLRTTrackParticles",
                                        "ExtraPolatedMuonsLRTTrackParticles",
                                        "MSOnlyExtraPolatedMuonsLRTTrackParticles",
                                        "CombinedStauTrackParticles",
                                        "AntiKt4EMTopoJets",
                                        "egammaClusters",
                                        "ElectronRingSets",
                                        "ElectronCaloRings",
                                        "JetRingSets",
                                        "JetCaloRings",
                                        "SlowMuons",
                                        #"LCOriginTopoClusters",
                                        "EMOriginTopoClusters",
                                        "Staus",
                                        "METAssoc_AntiKt4EMTopo",
                                        "MET_Core_AntiKt4EMTopo",
                                        "METAssoc_AntiKt4EMPFlow",
                                        "MET_Core_AntiKt4EMPFlow",
                                        "InDetLowPtRoITrackParticles",
                                        "PixelClusters",
                                        "PixelMSOSs",
                                        "DisappearingPixelMSOSs",
                                        "LowPtRoIPixelMSOSs",
                                        "SCT_Clusters",
                                        "SCT_MSOSs",
                                        "DisappearingSCT_MSOSs",
                                        "LowPtRoISCT_MSOSs",
                                        "LVL1MuonRoIs",
                                        ]


    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::JetContainer#AntiKt10EMTopoRCJets","xAOD::JetAuxContainer#AntiKt10EMTopoRCJetsAux.-PseudoJet"]
    StaticContent += ["CaloClusterCellLinkContainer#CaloCalTopoClusters_links"]

    for wp in LLP1VrtSecInclusiveSuffixes:
        StaticContent += ["xAOD::VertexContainer#VrtSecInclusive_SecondaryVertices" + wp]
        StaticContent += ["xAOD::VertexAuxContainer#VrtSecInclusive_SecondaryVertices" + wp + "Aux."]

    for wp in LLP1NewVSISuffixes:
        StaticContent += ["xAOD::VertexContainer#NewVrtSecInclusive_SecondaryVertices" + wp]
        StaticContent += ["xAOD::VertexAuxContainer#NewVrtSecInclusive_SecondaryVertices" + wp + "Aux." + excludedVertexAuxData]

    StaticContent += ["xAOD::VertexContainer#MuSAVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#MuSAVerticesAux."]
    StaticContent += ["xAOD::TrackParticleContainer#MuSAExtrapolatedTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#MuSAExtrapolatedTrackParticlesAux."]

    if flags.Derivation.LLP.doMuSAValidation:
        StaticContent += ["xAOD::VertexContainer#JPsiMuSAVertices"]
        StaticContent += ["xAOD::VertexAuxContainer#JPsiMuSAVerticesAux."]
        StaticContent += ["xAOD::VertexContainer#JPsiVertices"]
        StaticContent += ["xAOD::VertexAuxContainer#JPsiVerticesAux."]
        StaticContent += ["xAOD::TrackParticleContainer#JPsiVerticesTracks"]
        StaticContent += ["xAOD::TrackParticleAuxContainer#JPsiVerticesTracksAux."]
        StaticContent += ["xAOD::TrackParticleContainer#JPsiMuSAExtrapolatedTrackParticles"]
        StaticContent += ["xAOD::TrackParticleAuxContainer#JPsiMuSAExtrapolatedTrackParticlesAux."]
        StaticContent += ["xAOD::VertexContainer#ValidationMuSAVertices"]
        StaticContent += ["xAOD::VertexAuxContainer#ValidationMuSAVerticesAux."]
        StaticContent += ["xAOD::TrackParticleContainer#ValidationMuSAExtrapolatedTrackParticles"]
        StaticContent += ["xAOD::TrackParticleAuxContainer#ValidationMuSAExtrapolatedTrackParticlesAux."]

    LLP1SlimmingHelper.ExtraVariables += ["AntiKt10TruthTrimmedPtFrac5SmallR20Jets.Tau1_wta.Tau2_wta.Tau3_wta.D2.GhostBHadronsFinalCount",
                                          "Electrons.LHValue.DFCommonElectronsLHVeryLooseNoPixResult.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "LRTElectrons.LHValue.DFCommonElectronsLHVeryLooseNoPixResult.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "Photons.DFCommonPhotonsIsEMMedium.DFCommonPhotonsIsEMMediumIsEMValue.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "Muons.meanDeltaADCCountsMDT",
                                          "egammaClusters.phi_sampl.eta0.phi0",
                                          "LRTegammaClusters.phi_sampl.eta0.phi0",
                                          "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostBHadronsFinal.GhostCHadronsFinal.GhostTrack.GhostTrackCount.GhostTrackLRT.GhostTrackLRTCount.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z",
                                          "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.DFCommonJets_fJvt.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostBHadronsFinal.GhostCHadronsFinal.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z",
                                          "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201903.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostTausFinal.GhostTausFinalCount",
                                          "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201810.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostTausFinal.GhostTausFinalCount",
                                          "TruthPrimaryVertices.t.x.y.z.sumPt2",
                                          "PrimaryVertices.t.x.y.z.sumPt2.covariance",
                                          "InDetTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.patternRecoInfo",
                                          "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                          "InDetTrackParticles.numberOfIBLOverflowsdEdx.numberOfUsedHitsdEdx.pixeldEdx",
                                          "InDetTrackParticles.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberOfNextToInnermostPixelLayerHits.numberOfContribPixelLayers.numberOfGangedFlaggedFakes.numberOfPixelOutliers.numberOfPixelSplitHits.numberOfPixelSpoiltHits",
                                          "InDetTrackParticles.numberOfSCTOutliers.numberOfSCTSpoiltHits",
                                          "InDetTrackParticles.numberOfTRTHoles.numberOfTRTDeadStraws.numberOfTRTSharedHits.numberOfTRTHighThresholdHits.numberOfTRTHighThresholdHitsTotal.numberOfTRTHighThresholdOutliers.TRTdEdx.TRTdEdxUsedHits.hitPattern",
                                          "InDetTrackParticles.truthMatchProbability.truthOrigin.truthType",
                                          "InDetTrackParticles.TrkIsoPtPdEdx_ptcone20.TrkIsoPtPdEdx_ptcone30.TrkIsoPtPdEdx_ptcone40.TrkIsoPtTightPdEdx_ptcone20.TrkIsoPtTightPdEdx_ptcone30.TrkIsoPtTightPdEdx_ptcone40",
                                          "InDetTrackParticles.LLP1_ptcone20.LLP1_ptcone30.LLP1_ptcone40.LLP1_ptvarcone20.LLP1_ptvarcone30.LLP1_ptvarcone40.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag",
                                          "InDetTrackParticles.LLP1_topoetcone20.LLP1_topoetcone30.LLP1_topoetcone40.LLP1_topoetcone20NonCoreCone.LLP1_topoetcone30NonCoreCone.LLP1_topoetcone40NonCoreCone",
                                          "InDetTrackParticles.LLP1_CaloCelldEta.LLP1_CaloCelldPhi.LLP1_CaloCelldR.LLP1_CaloCelldX.LLP1_CaloCelldY.LLP1_CaloCelldZ.LLP1_CaloCellE.LLP1_CaloCellEta.LLP1_CaloCellGain.LLP1_CaloCellID.LLP1_CaloCellPhi.LLP1_CaloCellProvenance.LLP1_CaloCellQuality.LLP1_CaloCellR.LLP1_CaloCellSampling.LLP1_CaloCellTime.LLP1_CaloCellX.LLP1_CaloCellY.LLP1_CaloCellZ.LLP1_CaloCellEneDiff.LLP1_CaloCellTimeDiff",
                                          "InDetTrackParticles.Reco_msosLink",

                                          "InDetLargeD0TrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.patternRecoInfo",
                                          "GSFTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.hitPattern.truthOrigin.truthType",
                                          "LRTGSFTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.hitPattern.truthOrigin.truthType",
                                          "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                                          "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                          "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                                          "EventInfo.DFCommonJets_eventClean_SuperLooseBadLLP.DFCommonJets_eventClean_SuperLooseBadLLP_EMTopo.DFCommonJets_eventClean_LooseBadLLP_EMTopo",
                                          "TauJets.dRmax.etOverPtLeadTrk.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z",
                                          "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                          "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    # Isolation close by correction content for the LRT included corrections
    LLP1SlimmingHelper.ExtraVariables += ["Muons.topoetcone20_CloseByCorr_LRT.neflowisol20_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr_LRT",
                                          "MuonsLRT.topoetcone20_CloseByCorr_LRT.neflowisol20_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr_LRT",
                                          "Electrons.topoetcone20_CloseByCorr_LRT.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr_LRT",
                                          "LRTElectrons.topoetcone20_CloseByCorr_LRT.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr_LRT.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr_LRT",
                                          "Photons.topoetcone20_CloseByCorr_LRT.topoetcone40_CloseByCorr_LRT.ptcone20_CloseByCorr_LRT"
                                          ]

    VSITrackAuxVars = [
        "is_selected", "is_associated", "is_svtrk_final", "pt_wrtSV", "eta_wrtSV",
        "phi_wrtSV", "d0_wrtSV", "z0_wrtSV", "errP_wrtSV", "errd0_wrtSV",
        "errz0_wrtSV", "chi2_toSV"
    ]

    for suffix in LLP1VrtSecInclusiveSuffixes + LLP1NewVSISuffixes:
        LLP1SlimmingHelper.ExtraVariables += [ "InDetTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "InDetLargeD0TrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "GSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "LRTGSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]

    LLP1SlimmingHelper.ExtraVariables.append('CaloCalTopoClusters.e_sampl.calM.calE.calEta.calPhi.CENTER_MAG.SECOND_TIME.time')
    LLP1SlimmingHelper.AppendToDictionary["EMOriginTopoClusters"]='xAOD::CaloClusterContainer'
    LLP1SlimmingHelper.AppendToDictionary["EMOriginTopoClustersAux"]='xAOD::ShallowAuxContainer'
    LLP1SlimmingHelper.ExtraVariables.append('EMOriginTopoClusters.e_sampl.calM.calE.calEta.calPhi.CENTER_MAG.SECOND_TIME')

    
    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(LLP1SlimmingHelper)
        LLP1SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets', 'AntiKt4TruthJets']
        LLP1SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "LRTElectrons.TruthLink",
                                              "Muons.TruthLink",
                                              "MuonsLRT.TruthLink",
                                              "Photons.TruthLink"]

        if flags.Derivation.LLP.saveFullTruth:
            LLP1SlimmingHelper.ExtraVariables += ['TruthParticles', 'TruthVertices']
        StaticContent += ["xAOD::JetContainer#AntiKt10TruthRCJets","xAOD::JetAuxContainer#AntiKt10TruthRCJetsAux.-PseudoJet"]
    
    # ZeroPixelHitMuons container
    StaticContent += ["xAOD::MuonContainer#ZeroPixelHitMuons", "xAOD::MuonAuxContainer#ZeroPixelHitMuonsAux."]

    from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
        PhotonsCPDetailedContent,
        )
    LLP1SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(LLP1SlimmingHelper,writeLC=True,writeEM=True)
    LLP1SlimmingHelper.StaticContent = StaticContent

    # Trigger content
    LLP1SlimmingHelper.IncludeTriggerNavigation = False
    LLP1SlimmingHelper.IncludeJetTriggerContent = False
    LLP1SlimmingHelper.IncludeMuonTriggerContent = False
    LLP1SlimmingHelper.IncludeEGammaTriggerContent = False
    LLP1SlimmingHelper.IncludeTauTriggerContent = False
    LLP1SlimmingHelper.IncludeEtMissTriggerContent = False
    LLP1SlimmingHelper.IncludeBJetTriggerContent = False
    LLP1SlimmingHelper.IncludeBPhysTriggerContent = False
    LLP1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerMatchingToolRun2Cfg
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau)
        # Schedule additional pre-matching against LLP offline muons and electrons
        acc.merge(LLP1TriggerMatchingToolRun2Cfg(flags,
                                              name = "LRTTriggerMatchingTool_LLP1",
                                              OutputContainerPrefix = "LRTTrigMatch_LLP1_",
                                              TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau,
                                              InputElectrons=MergedElectronContainer,
                                              InputMuons=MergedMuonContainer_wZPH
                                              ))
        # And add the additional LLP trigger matching branches to the slimming helper 
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper,
                                               OutputContainerPrefix = "LRTTrigMatch_LLP1_",
                                               TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau,
                                               InputElectrons=MergedElectronContainer,
                                               InputMuons=MergedMuonContainer_wZPH
                                               )
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(LLP1SlimmingHelper)

    # Output stream
    LLP1ItemList = LLP1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_LLP1", ItemList=LLP1ItemList, AcceptAlgs=["LLP1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_LLP1", AcceptAlgs=["LLP1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc
