# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#====================================================================
# HIONHPOD.py
# Authors: Mariana Vivas <mariana.vivas.albornoz@cern.ch>, Ryan Jackson <r.d.jackson@cern.ch>
# Application: Open Data
#====================================================================
# Derivation for the heavy ion hard probes Open Data release.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

def HIONHPODGlobalAugmentationToolCfg(flags):
    """Global augmentation for track-based and calorimeter-based event-level information: Track multiplicity, 
    calorimeter energy sums and flow vectors"""
    acc = ComponentAccumulator()
    
    # Configure the augmentation tool
    augmentation_tool = CompFactory.DerivationFramework.HIGlobalAugmentationTool(name      = "HIONHPODAugmentationTool",
                                                                                 nHarmonic = 5) # to capture higher-order harmonics for anisotropic flow
    acc.addPublicTool(augmentation_tool, primary=True)

    return acc

def HIONHPODCentralityAugmentationToolCfg(flags):
    """Centrality augmentation: attaches centrality percentile boundaries to each event based on the measured FCal energy"""
    acc = ComponentAccumulator()

    # Configure centrality tool
    HICentralityDecorator = CompFactory.DerivationFramework.HICentralityDecorationTool(name="HIONHPODCentralityTool")
    
    # Add centrality tools to the ComponentAccumulator
    acc.addPublicTool(HICentralityDecorator, primary=True)

    return acc

def HIONHPODKernelCfg(flags, name='HIONHPODKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    GeV=1e3 # GeV -> MeV conversion factor

    from DerivationFrameworkInDet.InDetToolsConfig import (
        MuonTrackParticleThinningCfg,
        EgammaTrackParticleThinningCfg
    )
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_HITight_Cfg
    )
    
    # Initialize a list for all the different type of tools
    thinningTool = []
    augmentationTools = []

    HITightTrackSelector = acc.popToolsAndMerge(InDetTrackSelectionTool_HITight_Cfg(
        flags,
        name = "HIONHPODTrackSelectionToolTight",
        minPt = 10*GeV
    ))
    acc.addPublicTool(HITightTrackSelector)

    HIONHPODTrackThinningTool = CompFactory.DerivationFramework.HITrackParticleThinningTool(
        name = "HIONHPODTrackThinningTool",
        PrimaryVertexKey = "PrimaryVertices",
        PrimaryVertexSelection = "sumPt2",
        TrackSelectionTool = HITightTrackSelector,
        StreamName = kwargs["StreamName"]
    )

    acc.addPublicTool(HIONHPODTrackThinningTool)
    thinningTool += [HIONHPODTrackThinningTool]

    for jetKey in ("AntiKt2HIJets","AntiKt4HIJets"):    
        HIONHPODJetTrackThinningTool = CompFactory.DerivationFramework.HIJetTrackParticleThinningTool(
            name = f"HIONHPOD{jetKey}TrackThinningTool",
            PrimaryVertexKey = "PrimaryVertices",
            PrimaryVertexSelection = "sumPt2",
            JetKey = jetKey,
            TrackSelectionTool = HITightTrackSelector,
            StreamName = kwargs["StreamName"]
        )

        acc.addPublicTool(HIONHPODJetTrackThinningTool)
        thinningTool += [HIONHPODJetTrackThinningTool]

    # Muon thinning
    muonThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                   = "HIONHPODMuonThinningTool",
        StreamName             = kwargs['StreamName'], 
        MuonKey                = "Muons",
        InDetTrackParticlesKey = "InDetTrackParticles"
    ))

    acc.addPublicTool(muonThinningTool)
    thinningTool += [muonThinningTool]

    # Electron/photon thinning
    egamma_thinning_config = {
        "Electrons": {
        },
        "Photons": {
            "GSFConversionVerticesKey": "GSFConversionVertices"
        }
    }
    for egammaKey in egamma_thinning_config:
        egammaThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
            flags,
            name       = f"HIONHPOD{egammaKey}ThinningTool",
            StreamName = kwargs["StreamName"],
            SGKey      = egammaKey,
            **egamma_thinning_config[egammaKey]
        ))
        acc.addPublicTool(egammaThinningTool)
        thinningTool += [egammaThinningTool]

    # Merge the augmentation tools to the ComponetAccumlator
    globalAugmentationTool = acc.getPrimaryAndMerge(HIONHPODGlobalAugmentationToolCfg(flags))
    augmentationTools += [globalAugmentationTool]

    centralityAugmentationTool = acc.getPrimaryAndMerge(HIONHPODCentralityAugmentationToolCfg(flags))
    augmentationTools += [centralityAugmentationTool]

    for tool in augmentationTools:
        acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(tool.name+"Aug", AugmentationTools = [tool]))
    acc.addEventAlgo(
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name,
        ThinningTools=thinningTool
    ))

    return acc

def HIONHPODCfg(flags):
    acc = ComponentAccumulator()
    acc.merge(HIONHPODKernelCfg(flags, name="HIONHPODKernel", StreamName="StreamDAOD_HIONHPOD"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    ################################### Slimming ###################################
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    HIONHPODSlimmingHelper = SlimmingHelper("HIONHPODSlimmingHelper", NamesAndTypes=flags.Input.TypedCollections, flags=flags)
    
    # ListSlimming details all variable slimming configurations
    from DerivationFrameworkHI import ListSlimming

    # Smart collections: Empty
    HIONHPODSlimmingHelper.SmartCollections = ListSlimming.HIONHPODSmartCollections()

    # All variables: CaloSums
    HIONHPODSlimmingHelper.AllVariables += ListSlimming.HIONHPODAllVariables()

    # Extra variables: The bulk of the derivation. See ListSlimming.py for branches
    HIONHPODSlimmingHelper.ExtraVariables += ListSlimming.HIONHPODExtraVariablesAll()
    HIONHPODSlimmingHelper.ExtraVariables += ListSlimming.HIONHPODExtraVariablesJets()
    
    # Add truth information to Monte Carlo samples
    if flags.Input.isMC:
        HIONHPODSlimmingHelper.ExtraVariables += ListSlimming.HIONHPODExtraTruthVariables()
        HIONHPODSlimmingHelper.ExtraVariables += ListSlimming.HIONHPODExtraTruthVariablesJets()
        HIONHPODSlimmingHelper.AllVariables += ListSlimming.HIONHPODAllTruthVariables()

    HIONHPODItemList = HIONHPODSlimmingHelper.GetItemList()
    
    acc.merge(OutputStreamCfg(flags, "DAOD_HIONHPOD", ItemList=HIONHPODItemList, AcceptAlgs=["HIONHPODKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HIONHPOD", AcceptAlgs=["HIONHPODKernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
