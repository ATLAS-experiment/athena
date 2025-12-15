# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY4.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY4"
streamName = "StreamDAOD_BPHY4"

def BPHY4Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)

    BPHY4FourMuonTool = CompFactory.DerivationFramework.FourMuonTool(
            name                        = "BPHY4FourMuonTool",
            ptCut                       = 2500.0,
            etaCut                      = 2.5,
            muonCollectionKey           = "Muons",
            TrackParticleCollection     = "InDetTrackParticles",
            V0VertexFitterTool          = None,             # V0 vertex fitter
            useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
            TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
            TrackSelectorTool           = trackselect)
    BPHY4_Reco_4mu = CompFactory.DerivationFramework.Reco_4mu(
            name                    = "BPHY4_Reco_4mu",
            FourMuonTool            = BPHY4FourMuonTool,
            V0Tools                 = V0Tools,
            PVRefitter              = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            PairContainerName       = "BPHY4Pairs",
            QuadrupletContainerName = "BPHY4Quads",
            PVContainerName         = "PrimaryVertices",
            RefPVContainerName      = "BPHY4RefittedPrimaryVertices",
            RefitPV                 = True,
            MaxPVrefit              = 100000,
            DoVertexType            = 7)

    BPHY4MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name     = "BPHY4MuonTPThinningTool",
                                                                         MuonKey    = "Muons",
                                                                         StreamName = streamName,
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
    BPHY4ElectronTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(name     = "BPHY4ElectronTPThinningTool",
                                                                               SGKey                   = "Electrons",
                                                                               GSFTrackParticlesKey    = "GSFTrackParticles",
                                                                               StreamName = streamName,
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles")

    # Skim events accepted by the muon selection
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    BPHY4_SelectEvent = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = 'BPHY4_SelectEvent',
        expression = '(count(Muons.BPHY4MuonIndex>=0)>0)'))

    BPHY4ThinningTools = [BPHY4MuonTPThinningTool, BPHY4ElectronTPThinningTool]
    BPHY4SlimTools     = [BPHY4_SelectEvent]
    BPHY4AugTools      = [BPHY4_Reco_4mu]
    for t in BPHY4ThinningTools + BPHY4SlimTools + BPHY4AugTools: acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY4Kernel", 
                AugmentationTools= BPHY4AugTools,  SkimmingTools     = BPHY4SlimTools,  ThinningTools     = BPHY4ThinningTools  ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY4SlimmingHelper = SlimmingHelper("BPHY4SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY4AllVariables  = getDefaultAllVariables()
    BPHY4StaticContent = []
    BPHY4SmartVariables =[]
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    # Needed for trigger objects
    BPHY4SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY4SlimmingHelper.IncludeBPhysTriggerContent = True
    
    ## primary vertices
    BPHY4AllVariables += ["PrimaryVertices"]
    BPHY4StaticContent += ["xAOD::VertexContainer#BPHY4RefittedPrimaryVertices"]
    BPHY4StaticContent += ["xAOD::VertexAuxContainer#BPHY4RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    BPHY4AllVariables += ["InDetTrackParticles"]
    BPHY4SmartVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    BPHY4AllVariables += ["CombinedMuonTrackParticles"]
    BPHY4AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    BPHY4SmartVariables += ["Muons"]
    
    ## Electron container
    BPHY4SmartVariables += ["Electrons"] 
    
    ## Pair/quad candidates 
    BPHY4StaticContent += ["xAOD::VertexContainer#%s"        % BPHY4_Reco_4mu.PairContainerName]
    BPHY4StaticContent += ["xAOD::VertexContainer#%s"        % BPHY4_Reco_4mu.QuadrupletContainerName]
    
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY4StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY4_Reco_4mu.PairContainerName]
    BPHY4StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY4_Reco_4mu.QuadrupletContainerName]

    BPHY4SlimmingHelper.AllVariables = BPHY4AllVariables
    BPHY4SlimmingHelper.StaticContent = BPHY4StaticContent
    BPHY4SlimmingHelper.SmartCollections = BPHY4SmartVariables
    BPHY4ItemList = BPHY4SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY4", ItemList=BPHY4ItemList, AcceptAlgs=["BPHY4Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY4", AcceptAlgs=["BPHY4Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
