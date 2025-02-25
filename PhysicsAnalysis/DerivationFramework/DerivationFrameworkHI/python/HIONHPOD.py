# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# HIONHPOD.py
# author: Mariana Vivas <mariana.vivas.albornoz@cern.ch>
# Application: Open Data
#====================================================================

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

    from DerivationFrameworkInDet.InDetToolsConfig import JetTrackParticleThinningCfg
    
    # Initialize a list for all the different type of tools
    thinningTool = []
    augmentationTool = []

    # AntiKt4HI jets thinning
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
                                                                                    name                   = "AntiKt4HIJetsThinningTool",
                                                                                    StreamName             = kwargs['StreamName'],
                                                                                    JetKey                 = "AntiKt4HIJets",
                                                                                    InDetTrackParticlesKey = "InDetTrackParticles"))
    
    acc.addPublicTool(AntiKt4HIJetsThinningTool)
    thinningTool += [AntiKt4HIJetsThinningTool]

    # Muon thinning
    muonThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name                  = "HIONHPODMuonThinningTool",
                                                                                MuonKey                = "Muons",
                                                                                InDetTrackParticlesKey = "InDetTrackParticles")

    acc.addPublicTool(muonThinningTool)
    thinningTool += [muonThinningTool]

    # Merge the augmentation tools to the ComponetAccumlator
    globalAugmentationTool = acc.getPrimaryAndMerge(HIONHPODGlobalAugmentationToolCfg(flags))
    augmentationTool += [globalAugmentationTool]

    centralityAugmentatioTool = acc.getPrimaryAndMerge(HIONHPODCentralityAugmentationToolCfg(flags))
    augmentationTool += [centralityAugmentatioTool]

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                    ThinningTools=thinningTool,
                                    AugmentationTools=augmentationTool
                                    ),
                                )

    return acc

def HIONHPODCfg(flags):
    acc = ComponentAccumulator()
    acc.merge(HIONHPODKernelCfg(flags, name="HIONHPODKernel", StreamName="StreamDAOD_HIONHPOD"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    ################################### Slimming ###################################
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    HIONHPODSlimmingHelper = SlimmingHelper("HIONHPODSlimmingHelper", NamesAndTypes=flags.Input.TypedCollections, flags=flags)
    
    from DerivationFrameworkHI import ListSlimming
    # Only smart collection variables for electrons, muons and photons
    HIONHPODSlimmingHelper.SmartCollections = ListSlimming.HIONHPODSmartCollections()
    # And all the variables for AntiKt4HIJets, CaloSums and EventInfo
    HIONHPODSlimmingHelper.AllVariables = ["AntiKt4HIJets",
                                           "CaloSums",
                                           "EventInfo"]

    # Add truth information to Monte Carlo samples
    if flags.Input.isMC:
        from SGComps.AddressRemappingConfig import AddressRemappingCfg
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddStandardTruthContentsCfg
        
        # Define required container names as encoded bytes
        required_containers = { "AntiKt10TruthJets".encode("utf-8"), "AntiKt10TruthJetsAux".encode("utf-8") }

        # No need to decode the collections as they are all bytes
        inputCollections = set(flags.Input.Collections)

        if inputCollections.intersection(required_containers):
            rename_maps = ['%s#%s->%s' % ("xAOD::JetContainer", "AntiKt10TruthJets", "old_AntiKt10TruthJets"),
                           '%s#%s->%s' % ("xAOD::JetAuxContainer", "AntiKt10TruthJetsAux.", "old_AntiKt10TruthJetsAux.")
                         ]
            acc.merge(AddressRemappingCfg(rename_maps))
    
        acc.merge(AddStandardTruthContentsCfg(flags))

        HIONHPODSlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD::EventAuxInfo',
                                                'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                                'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                                'TruthLHEParticles':'xAOD::TruthParticleContainer', 'TruthLHEParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBottom':'xAOD::TruthParticleContainer','TruthBottomAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
                                                'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                'AntiKt4TruthDressedWZJets':'xAOD::JetContainer','AntiKt4TruthDressedWZJetsAux':'xAOD::JetAuxContainer',
                                                'AntiKt10TruthSoftDropBeta100Zcut10Jets':'xAOD::JetContainer','AntiKt10TruthSoftDropBeta100Zcut10JetsAux':'xAOD::JetAuxContainer'
                                             }

    # Add standard content
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
    addTruth3ContentToSlimmerTool(HIONHPODSlimmingHelper)

    HIONHPODItemList = HIONHPODSlimmingHelper.GetItemList()
    
    acc.merge(OutputStreamCfg(flags, "DAOD_HIONHPOD", ItemList=HIONHPODItemList, AcceptAlgs=["HIONHPODKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HIONHPOD", AcceptAlgs=["HIONHPODKernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
