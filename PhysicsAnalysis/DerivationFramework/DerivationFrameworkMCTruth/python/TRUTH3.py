# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# TRUTH3.py - format containing standard common ATLAS truth record

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import MetadataCategory

def TRUTH3Cfg(flags):
    """Main config for TRUTH3"""
    acc = ComponentAccumulator()

    # Add translator from EVGEN input to xAOD-like truth
    # Add the truth content of DAOD_PHYS
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTruthDerivationContentsCfg
    acc.merge(AddTruthDerivationContentsCfg(flags))

    #==============================================================================
    # Set up slimming content list
    #==============================================================================
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TRUTH3SlimmingHelper = SlimmingHelper("TRUTH3SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    TRUTH3SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD::EventAuxInfo',
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

    # Add EventInfo
    TRUTH3SlimmingHelper.AllVariables = [ 'EventInfo' ]

    # Add TruthTau collections
    TRUTH3SlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']

    # Add standard content
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import addPhysTruthContentToSlimmerTool
    addPhysTruthContentToSlimmerTool(TRUTH3SlimmingHelper)
        
    # Generate item list and create stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    TRUTH3ItemList = TRUTH3SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TRUTH3", ItemList=TRUTH3ItemList))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TRUTH3", createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc
