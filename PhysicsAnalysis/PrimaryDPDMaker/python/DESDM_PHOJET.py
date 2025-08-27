# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DESDM_PHOJET.py
# Component accumulator version
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be 
# run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


# Main algorithm config
def DESDM_PHOJETKernelCfg(configFlags, name='DESDM_PHOJETKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHOJET"""
    acc = ComponentAccumulator()

    #====================================================================
    # SKIMMING TOOLS 
    #====================================================================
    skimmingTools = []
    if not configFlags.Input.isMC:
        pho_sel = 'count(Photons.pt > 280*GeV && Photons.Tight) >= 1'

        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
        skimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, 
                                                                        name       = "DESDM_PHOJET_SkimmingTool",
                                                                        expression = pho_sel))
        skimmingTools.append(skimmingTool)
    
    PHOJETKernel = CompFactory.DerivationFramework.DerivationKernel(name, SkimmingTools = skimmingTools)
    acc.addEventAlgo( PHOJETKernel )

    return acc

# Main config
def DESDM_PHOJETCfg(configFlags):
    """Main config fragment for DESDM_PHOJET"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(DESDM_PHOJETKernelCfg(configFlags, name="DESDM_PHOJETKernel", StreamName = 'StreamDESDM_PHOJET'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    items = ['xAOD::EventInfo#*', 'xAOD::EventAuxInfo#*',
             # Standard CP objects
             'xAOD::ElectronContainer#Electrons','xAOD::ElectronAuxContainer#ElectronsAux.',
             'xAOD::MuonContainer#Muons','xAOD::MuonAuxContainer#MuonsAux.',
             'xAOD::PhotonContainer#Photons','xAOD::PhotonAuxContainer#PhotonsAux.',
             'xAOD::TauJetContainer#TauJets','xAOD::TauJetAuxContainer#TauJetsAux.-VertexedClusters',
             'xAOD::VertexContainer#PrimaryVertices','xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV',
             'xAOD::JetContainer#AntiKt4EMPFlowJets','xAOD::JetAuxContainer#AntiKt4EMPFlowJetsAux.-PseudoJet',
             'xAOD::JetContainer#AntiKt4EMTopoJets','xAOD::JetAuxContainer#AntiKt4EMTopoJetsAux.-PseudoJet',
             'xAOD::EventShape#Kt4EMPFlowEventShape','xAOD::EventShapeAuxInfo#Kt4EMPFlowEventShapeAux.',
             'xAOD::EventShape#Kt4EMTopoOriginEventShape','xAOD::EventShapeAuxInfo#Kt4EMTopoOriginEventShapeAux.',
             'xAOD::MissingETAssociationMap#METAssoc_AntiKt4EMPFlow', 'xAOD::MissingETAuxAssociationMap#METAssoc_AntiKt4EMPFlowAux.',
             'xAOD::MissingETAssociationMap#METAssoc_AntiKt4EMTopo', 'xAOD::MissingETAuxAssociationMap#METAssoc_AntiKt4EMTopoAux.',
             'xAOD::MissingETContainer#MET_Core_AntiKt4EMPFlow','xAOD::MissingETAuxContainer#MET_Core_AntiKt4EMPFlowAux.',
             'xAOD::MissingETContainer#MET_Core_AntiKt4EMTopo','xAOD::MissingETAuxContainer#MET_Core_AntiKt4EMTopoAux.',
             'xAOD::MissingETContainer#MET_Reference_AntiKt4EMPFlow','xAOD::MissingETAuxContainer#MET_Reference_AntiKt4EMPFlowAux.',
             'xAOD::MissingETContainer#MET_Reference_AntiKt4EMTopo', 'xAOD::MissingETAuxContainer#MET_Reference_AntiKt4EMTopoAux.',
             'xAOD::MissingETContainer#MET_Track','xAOD::MissingETAuxContainer#MET_TrackAux.',
             #Low-level inputs:
             'xAOD::TrackParticleContainer#GSFTrackParticles','xAOD::TrackParticleAuxContainer#GSFTrackParticlesAux.',
             'xAOD::VertexContainer#GSFConversionVertices','xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex',
             'xAOD::TrackParticleContainer#InDetTrackParticles','xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.',
             'xAOD::CaloClusterContainer#egammaClusters','xAOD::CaloClusterAuxContainer#egammaClustersAux.',
             'xAOD::CaloClusterContainer#ForwardElectronClusters','xAOD::CaloClusterAuxContainer#ForwardElectronClustersAux.',
             'xAOD::CaloClusterContainer#TauPi0Clusters','xAOD::CaloClusterAuxContainer#TauPi0ClustersAux.',
             'xAOD::CaloClusterContainer#CaloCalTopoClusters','xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.',
             'xAOD::MuonSegmentContainer#MuonSegments','xAOD::MuonSegmentAuxContainer#MuonSegmentsAux.',
             'xAOD::FlowElementContainer#JetETMissChargedParticleFlowObjects','xAOD::FlowElementAuxContainer#JetETMissChargedParticleFlowObjectsAux.',
             'xAOD::FlowElementContainer#JetETMissNeutralParticleFlowObjects','xAOD::FlowElementAuxContainer#JetETMissNeutralParticleFlowObjectsAux.',
             'xAOD::TrackParticleClusterAssociationContainer#InDetTrackParticlesClusterAssociations','xAOD::TrackParticleClusterAssociationAuxContainer#InDetTrackParticlesClusterAssociationsAux.',
             'CaloCellContainer#AllCalo',
             'CaloClusterCellLinkContainer#CaloCalTopoClusters_links',
             'CaloClusterCellLinkContainer#InDetTrackParticlesAssociatedClusters_links',
             'CaloClusterCellLinkContainer#MuonClusterCollection_links',
             'CaloClusterCellLinkContainer#egammaClusters_links',
             #RoI Containers
             'xAOD::EmTauRoIContainer#LVL1EmTauRoIs','xAOD::EmTauRoIAuxContainer#LVL1EmTauRoIsAux.',
             'xAOD::EnergySumRoI#LVL1EnergySumRoI','xAOD::EnergySumRoIAuxInfo#LVL1EnergySumRoIAux.',
             'xAOD::JetEtRoI#LVL1JetEtRoI','xAOD::JetEtRoIAuxInfo#LVL1JetEtRoIAux.',
             'xAOD::JetRoIContainer#LVL1JetRoIs','xAOD::JetRoIAuxContainer#LVL1JetRoIsAux.',
             'xAOD::MuonRoIContainer#LVL1MuonRoIs','xAOD::MuonRoIAuxContainer#LVL1MuonRoIsAux.',
             #L1Calo containers
             'xAOD::JEMTobRoIContainer#JEMTobRoIsRoIB','xAOD::JEMTobRoIAuxContainer#JEMTobRoIsRoIBAux.',
             'xAOD::JEMTobRoIContainer#JEMTobRoIs','xAOD::JEMTobRoIAuxContainer#JEMTobRoIsAux.',
             'xAOD::JEMEtSumsContainer#JEMEtSums','xAOD::JEMEtSumsAuxContainer#JEMEtSumsAux.',
             'xAOD::CMXCPHitsContainer#CMXCPHits','xAOD::CMXCPHitsAuxContainer#CMXCPHitsAux.',
             'xAOD::CMXCPTobContainer#CMXCPTobs','xAOD::CMXCPTobAuxContainer#CMXCPTobsAux.',
             'xAOD::CMXEtSumsContainer#CMXEtSums','xAOD::CMXEtSumsAuxContainer#CMXEtSumsAux.',
             'xAOD::CMXJetHitsContainer#CMXJetHits','xAOD::CMXJetHitsAuxContainer#CMXJetHitsAux.',
             'xAOD::CMXJetTobContainer#CMXJetTobs','xAOD::CMXJetTobAuxContainer#CMXJetTobsAux.',
             'xAOD::CMXRoIContainer#CMXRoIs','xAOD::CMXRoIAuxContainer#CMXRoIsAux.',
             'xAOD::CPMTobRoIContainer#CPMTobRoIs','xAOD::CPMTobRoIAuxContainer#CPMTobRoIsAux.',
             'xAOD::CPMTobRoIContainer#CPMTobRoIsRoIB','xAOD::CPMTobRoIAuxContainer#CPMTobRoIsRoIBAux.',
             'xAOD::CPMTowerContainer#CPMTowers','xAOD::CPMTowerAuxContainer#CPMTowersAux.',
             'xAOD::CPMTowerContainer#CPMTowersOverlap','xAOD::CPMTowerAuxContainer#CPMTowersOverlapAux.',
             'xAOD::RODHeaderContainer#RODHeaders','xAOD::RODHeaderAuxContainer#RODHeadersAux.',
             'xAOD::JetElementContainer#JetElements','xAOD::JetElementAuxContainer#JetElementsAux.',
             'xAOD::JetElementContainer#JetElementsOverlap','xAOD::JetElementAuxContainer#JetElementsOverlapAux.',
             'xAOD::TriggerTowerContainer#xAODTriggerTowers','xAOD::TriggerTowerAuxContainer#xAODTriggerTowersAux.',
             'xAOD::L1TopoRawDataContainer#L1TopoRawData','xAOD::L1TopoRawDataAuxContainer#L1TopoRawDataAux.',
             #Trigger navigation
             'xAOD::TrigDecision#*','xAOD::TrigDecisionAuxInfo#*',
             'xAOD::TrigConfKeys#*',
             'xAOD::BunchConfKey#*',
             'xAOD::TrigNavigation#*','xAOD::TrigNavigationAuxInfo#*',
             'xAOD::TrigCompositeContainer#HLTNav*', 'xAOD::TrigCompositeAuxContainer#HLTNav*'
             ]

    if configFlags.Input.isMC:
        items += ['xAOD::TruthParticleContainer#*','xAOD::TruthParticleAuxContainer#TruthParticlesAux.-caloExtension',
                  'xAOD::TruthVertexContainer#*','xAOD::TruthVertexAuxContainer#*',
                  'xAOD::TruthEventContainer#*','xAOD::TruthEventAuxContainer#*']

    acc.merge( OutputStreamCfg( configFlags, 'DESDM_PHOJET', ItemList=items, AcceptAlgs=["DESDM_PHOJETKernel"]) )
    acc.merge(
        SetupMetaDataForStreamCfg(
            configFlags,
            "DESDM_PHOJET",
            AcceptAlgs=["DESDM_PHOJETKernel"],
            createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TriggerMenuMetaData,
            ],
        )
    )

    return acc


