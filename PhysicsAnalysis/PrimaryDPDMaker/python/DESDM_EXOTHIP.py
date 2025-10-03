## Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#=======================================================================

# DESDM_EXOTHIP.py
# Component accumulator version
# Author : Priyanka Kumari (pkumari@cern.ch)
#=======================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


# Main algorithm config
def DESDM_EXOTHIPKernelCfg(flags, name='DESDM_EXOTHIPKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for HIPTrigger"""
    acc = ComponentAccumulator()

    #====================================================================
    # SKIMMING TOOLS 
    #====================================================================

    if not flags.Input.isMC:
        hip_trigger = '(HLT_g0_hiptrt_L1eEM26M || HLT_g0_hiptrt_L1eEM28M)'
        
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
        stringSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                              name       = "DESDM_EXOTHIP_SkimmingTool",
                                                                              expression = hip_trigger))
        skimTool1 = stringSkimmingTool

        CPPSkimmingTool =  CompFactory.DerivationFramework.DESDM_EXOTHIP_SkimmingTool(name = "DESDM_SkimmingCPPTool",
                                                                                  HTTRTHitsCounter = "HLT_TrigTRTHTCounts",
                                                                                  MinHTRatioWedge  = 0.5)
        acc.addPublicTool(CPPSkimmingTool,primary =  True)
        skimTool2 = CPPSkimmingTool


        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationANDCfg
        CombinedTool = acc.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                                      name="HIPtrrigerTool",
                                                                      FilterList=[skimTool1, skimTool2]))
        acc.addPublicTool(CombinedTool,primary = True)
        
    EXOTHIPKernel = CompFactory.DerivationFramework.DerivationKernel(name, SkimmingTools = [CombinedTool])
    acc.addEventAlgo( EXOTHIPKernel )

    return acc

# Main config
def DESDM_EXOTHIPCfg(flags):
    """Main config fragment for DESDM_EXOTHIP"""
    acc = ComponentAccumulator()
    
    # Main algorithm (kernel)
    acc.merge(DESDM_EXOTHIPKernelCfg(flags, name="DESDM_EXOTHIPKernel", StreamName = 'StreamDESDM_EXOTHIP'))
    
    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    
    container_items = ['xAOD::EventInfo#*', 'xAOD::EventAuxInfo#*',
                       
                       'xAOD::TrigNavigation#TrigNavigation','xAOD::TrigNavigationAuxInfo#TrigNavigationAux.',
                       'xAOD::TrigDecision#xTrigDecision','xAOD::TrigDecisionAuxInfo#xTrigDecisionAux.',
                       'xAOD::TrigConfKeys#TrigConfKeys',
                       'xAOD::TrigRNNOutputContainer#HLT_TrigTRTHTCounts','xAOD::TrigRNNOutputAuxContainer#HLT_TrigTRTHTCountsAux.',
                       'xAOD::TrigRNNOutputContainer#HLT_TrigRingerNeuralFex','xAOD::TrigRNNOutputAuxContainer#HLT_TrigRingerNeuralFexAux.',
                       'xAOD::CaloClusterContainer#egammaClusters','xAOD::CaloClusterAuxContainer#egammaClustersAux.',
                       'xAOD::CaloClusterContainer#CaloCalTopoClusters','xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.',
                       'xAOD::CaloClusterContainer#InDetTrackParticlesAssociatedClusters','xAOD::CaloClusterAuxContainer#InDetTrackParticlesAssociatedClustersAux.',
                       'xAOD::CaloClusterContainer#LArClusterEM','xAOD::CaloClusterAuxContainer#LArClusterEMAux.',
                       'xAOD::CaloClusterContainer#LArClusterEM7_11Nocorr','xAOD::CaloClusterAuxContainer#LArClusterEM7_11NocorrAux.',
                       'xAOD::CaloClusterContainer#egammaTopoSeededClusters','xAOD::CaloClusterAuxContainer#egammaTopoSeededClustersAux.',
                       
                       'CaloCellContainer#AllCalo',
                       'CaloClusterCellLinkContainer#InDetTrackParticlesAssociatedClusters_links',
                       'CaloClusterCellLinkContainer#LArClusterEM7_11Nocorr_links',
                       'CaloClusterCellLinkContainer#LArClusterEM_links',
                       'CaloClusterCellLinkContainer#CaloCalTopoClusters_links',
                       'CaloClusterCellLinkContainer#egammaClusters_links',
                       'CaloClusterCellLinkContainer#egammaTopoSeededClusters_links',
                       
                       'InDet::TRT_DriftCircleContainer#TRT_DriftCircles',
                       'LArDigitContainer#LArDigitContainer_EMClust',
                       'LArDigitContainer#LArDigitContainer_Thinned',

                       'xAOD::VertexContainer#PrimaryVertices','xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV',
                       'xAOD::PhotonContainer#Photons','xAOD::PhotonAuxContainer#PhotonsAux.'
                       ]
    if flags.Input.isMC:
        container_items += ['xAOD::TruthParticleContainer#*','xAOD::TruthParticleAuxContainer#TruthParticlesAux.-caloExtension',
                            'xAOD::TruthVertexContainer#*','xAOD::TruthVertexAuxContainer#*',
                            'xAOD::TruthEventContainer#*','xAOD::TruthEventAuxContainer#*']
        
    acc.merge( OutputStreamCfg( flags, 'DESDM_EXOTHIP', ItemList=container_items, AcceptAlgs=["DESDM_EXOTHIPKernel"]) )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DESDM_EXOTHIP",
            AcceptAlgs=["DESDM_EXOTHIPKernel"],
            createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.CutFlowMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TriggerMenuMetaData,
            ],
        )
    )
    
    return acc

                       
                       
    



