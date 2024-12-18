# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# HION14.py
# author: Mariana Vivas <mariana.vivas.albornoz@cern.ch>
# Application: Open Data
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

#Skiming
def HION14SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    
    #Building jet skimming triggers
    triggers  = ["HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50","HLT_noalg_mb_L1TE50"]
    
    expression = ' ( ' +' || '.join(triggers) + ' )'
    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION14StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                                                                             primary = True) 
    return(acc)

def HION14GlobalAugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()
    
    # Configure the augmentation tool
    # This adds FCalEtA, FCalEtC, ...
    augmentation_tool = CompFactory.DerivationFramework.HIGlobalAugmentationTool(name="HION14AugmentationTool",
                                                                                nHarmonic=5 # to capture higher-order harmonics for anisotropic flow
                                                                                )
    acc.addPublicTool(augmentation_tool, primary=True)

    return acc

def HION14TightAugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()

    # Configure track selection tools
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HITight_Cfg 

    HITightTrackSelector = acc.popToolsAndMerge(InDetTrackSelectionTool_HITight_Cfg(flags,
                                                                                    name="HITightTrackSelector",
                                                                                    minPt=100
                                                                                    )
                                                                                )

    # Add track selection tools to the ComponentAccumulator
    acc.addPublicTool(HITightTrackSelector)

    # Adding the decoration for HITight
    HITightDecorator = CompFactory.DerivationFramework.InDetTrackSelectionToolWrapper(name='HION14TighDecorator', 
                                                                                    TrackSelectionTool=HITightTrackSelector, 
                                                                                    DecorationName='HITight', 
                                                                                    ContainerName="InDetTrackParticles"
                                                                                    )
    
    # Merge the ComponentAccumulator returned by the decorator configuration
    acc.addPublicTool(HITightDecorator, primary=True)

    return acc

def HION14CentralityAugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()

    # Centrality tool
    HICentralityDecorator = CompFactory.DerivationFramework.HICentralityDecorationTool(name="HION14CentralityTool")
    
    # Add centrality tools to the ComponentAccumulator
    acc.addPublicTool(HICentralityDecorator, primary=True)

    return acc

def HION14KernelCfg(flags, name='HION14Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    skimmingTool = []
    # Only apply the triggers to data (it doesn't work on MC for a unknown reason)
    if not flags.Input.isMC:
        triggers = acc.getPrimaryAndMerge(HION14SkimmingToolCfg(flags))
        skimmingTool += [triggers]

    ################################### Thinning ###################################
    thinningTool = []

    # Loose thinning 
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HILoose_Cfg
    
    HILooseTrackSelector = acc.popToolsAndMerge(InDetTrackSelectionTool_HILoose_Cfg(flags,
                                                            name = "HION14TrackSelectionToolLoose",
                                                            minPt = 100
                                                            )
                                                        )

    acc.addPublicTool(HILooseTrackSelector)

    HION14TrackThinningTool = CompFactory.DerivationFramework.HITrackParticleThinningTool(name="HION14TrackThinningTool",
                                                                                        #InDetTrackParticlesKey="InDetTrackParticles",
                                                                                        PrimaryVertexKey="PrimaryVertices",
                                                                                        PrimaryVertexSelection="sumPt2",
                                                                                        TrackSelectionTool=HILooseTrackSelector
                                                                                        )

    acc.addPublicTool(HION14TrackThinningTool)
    thinningTool += [HION14TrackThinningTool]

    # Muon thinning
    muonThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name="HION14MuonThinningTool",
                                                                                MuonKey = "Muons",
                                                                                InDetTrackParticlesKey = "InDetTrackParticles")

    acc.addPublicTool(muonThinningTool)
    thinningTool += [muonThinningTool]

    # Truth thinning
    if flags.Input.isMC:
        truth_thinning_expression = "(TruthParticles.status == 1) && ( (TruthParticles.pdgId != 2112 && TruthParticles.pdgId != 2212) || TruthParticles.pt > 0.1 )"
        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg

        HION14TruthThinningTool = acc.getPrimaryAndMerge(GenericTruthThinningCfg(flags,
                                                                                name="HION14TruthThinningTool",
                                                                                StreamName=kwargs['StreamName'],
                                                                                ParticleSelectionString=truth_thinning_expression
                                                                                )
                                                                            )
        
        thinningTool += [HION14TruthThinningTool]

    ################################################################################
    # Merge the augmentation tools to the ComponetAccumlator
    globalAugmentationTool = acc.getPrimaryAndMerge(HION14GlobalAugmentationToolCfg(flags))
    tightAugmentationTool = acc.getPrimaryAndMerge(HION14TightAugmentationToolCfg(flags))
    centralityAugmentatioTool = acc.getPrimaryAndMerge(HION14CentralityAugmentationToolCfg(flags))
    augmentationTool = [globalAugmentationTool, tightAugmentationTool, centralityAugmentatioTool]

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                    SkimmingTools= skimmingTool,
                                    ThinningTools=thinningTool,
                                    AugmentationTools=augmentationTool
                                    ),
                                )

    return acc

def HION14Cfg(flags):
    acc = ComponentAccumulator()
    acc.merge(HION14KernelCfg(flags, name="HION14Kernel", StreamName="StreamDAOD_HION14"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    ################################### Slimming ###################################
    from DerivationFrameworkHI import ListSlimming

    HION14SlimmingHelper = SlimmingHelper("HION14SlimmingHelper", NamesAndTypes=flags.Input.TypedCollections, flags=flags)

    HION14SlimmingHelper.SmartCollections = ListSlimming.HION14SmartCollections()
    # For these variables we want all the branches
    HION14SlimmingHelper.AllVariables = ListSlimming.HION14AllVariablesGeneral()
    # These are selected branches
    HION14SlimmingHelper.ExtraVariables = ListSlimming.HION14ExtraContentAll()

    # Truth information
    if flags.Input.isMC:
        HION14SlimmingHelper.ExtraVariables +=ListSlimming.HION14ExtraContentAllTruth()
        HION14SlimmingHelper.AllVariables += ListSlimming.HION14TruthVariablesGeneral()


    HION14ItemList = HION14SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION14", ItemList=HION14ItemList, AcceptAlgs=["HION14Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION14", AcceptAlgs=["HION14Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

