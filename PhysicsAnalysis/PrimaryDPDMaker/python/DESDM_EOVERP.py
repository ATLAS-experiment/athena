# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DESDM_EOVERP.py
# IMPORTANT: this is NOT an AOD based derived data type but one built
# from ESD. It consequently has to be run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def DESDM_EOVERPKernelCfg(configFlags, name='DESDM_EOVERPKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for EOVERP"""
    acc = ComponentAccumulator()

    # Skimming
    desd_trig = '(HLT_mb_sptrk)'
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (xAODStringSkimmingToolCfg)
    skimmingTool =  xAODStringSkimmingToolCfg(
        flags, name = "DESDM_EOVERP_SkimmingTool",
        expression = desd_trig)

    # Kernel algorithm
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]))

    return acc

# Main config
def DESDM_EOVERPCfg(configFlags):
    """Main config fragment for DESDM_EOVERP"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(DESDM_EOVERPKernelCfg(configFlags, name="DESDM_EOVERPKernel", StreamName = 'StreamDESDM_EOVERP'))

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
             'xAOD::CaloClusterContainer#ForwardElectronClusters','xAOD::CaloClusterAuxContainer#ForwardElectronClustersAux.-sigmaWidth',
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

    acc.merge( OutputStreamCfg( configFlags, 'DESDM_EOVERP', ItemList=items, AcceptAlgs=["DESDM_EOVERPKernel"]) )

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    acc.merge(
        SetupMetaDataForStreamCfg(
            configFlags,
            "DESDM_EOVERP",
            AcceptAlgs=["DESDM_EOVERPKernel"],
            createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TriggerMenuMetaData,
            ],
        )
    )

    return acc


