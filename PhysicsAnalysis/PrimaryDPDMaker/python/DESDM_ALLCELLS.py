# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DESDM_ALLCELLS.py
# Component accumulator version
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be 
# run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def DESDM_ALLCELLSKernelCfg(configFlags, name='DESDM_ALLCELLSKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for ALLCELLS"""
    acc = ComponentAccumulator()

    # ALLCELLS has no skimming
    ALLCELLSKernel = CompFactory.DerivationFramework.DerivationKernel(name)
    acc.addEventAlgo( ALLCELLSKernel )

    return acc

# Main config
def DESDM_ALLCELLSCfg(configFlags):
    """Main config fragment for DESDM_ALLCELLS"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(DESDM_ALLCELLSKernelCfg(configFlags, name="DESDM_ALLCELLSKernel", StreamName = 'StreamDESDM_ALLCELLS'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    items = ['xAOD::EventInfo#*', 'xAOD::EventAuxInfo#*',
             # trigger content
             'xAOD::TrigDecision#xTrigDecision', 'xAOD::TrigDecisionAuxInfo#xTrigDecisionAux.',
             'xAOD::TrigCompositeContainer#HLTNav_Summary_AODSlimmed', 'xAOD::TrigCompositeAuxContainer#HLTNav_Summary_AODSlimmedAux.',
             'xAOD::TrigConfKeys#TrigConfKeys', 
             'xAOD::BunchConfKey#BunchConfKey',
             # Standard CP objects
             'xAOD::ElectronContainer#Electrons','xAOD::ElectronAuxContainer#ElectronsAux.',
             'xAOD::PhotonContainer#Photons','xAOD::PhotonAuxContainer#PhotonsAux.',
             'xAOD::VertexContainer#PrimaryVertices','xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV',
             'xAOD::TrackParticleContainer#GSFTrackParticles','xAOD::TrackParticleAuxContainer#GSFTrackParticlesAux.',
             'xAOD::VertexContainer#GSFConversionVertices','xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex',
             'xAOD::TrackParticleContainer#InDetTrackParticles','xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.',
             'xAOD::CaloClusterContainer#egammaClusters','xAOD::CaloClusterAuxContainer#egammaClustersAux.',
             'xAOD::CaloClusterContainer#ForwardElectronClusters','xAOD::CaloClusterAuxContainer#ForwardElectronClustersAux.',
             'xAOD::CaloClusterContainer#CaloCalTopoClusters','xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.',
             'CaloCellContainer#AllCalo',
             'CaloClusterCellLinkContainer#CaloCalTopoClusters_links',
             'CaloClusterCellLinkContainer#egammaClusters_links',
             'CaloClusterCellLinkContainer#ForwardElectronClusters_links'
             ]

    if configFlags.Input.isMC:
        items += ['xAOD::TruthParticleContainer#*','xAOD::TruthParticleAuxContainer#TruthParticlesAux.-caloExtension',
                  'xAOD::TruthVertexContainer#*','xAOD::TruthVertexAuxContainer#*',
                  'xAOD::TruthEventContainer#*','xAOD::TruthEventAuxContainer#*']

    acc.merge( OutputStreamCfg( configFlags, 'DESDM_ALLCELLS', ItemList=items, AcceptAlgs=["DESDM_ALLCELLSKernel"]) )

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    acc.merge(
        SetupMetaDataForStreamCfg(
            configFlags,
            "DESDM_ALLCELLS",
            AcceptAlgs=["DESDM_ALLCELLSKernel"],
            createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TriggerMenuMetaData,
            ],
        )
    )

    return acc


