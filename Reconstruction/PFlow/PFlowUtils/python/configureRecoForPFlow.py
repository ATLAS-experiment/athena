# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

#This function adds data to the output AOD which are needed for pflow CP study workflows.
#Currently it only adds data required to run jet finding from the AOD.
#If the needed data is not produced in the main reco job, we also configure
#it here to run.
def addContainersForPFlowCPStudiesCfg(cfgFlags):

    cfg = ComponentAccumulator()

    from CaloRec.CaloFwdTopoTowerConfig import CaloFwdTopoTowerCfg
    cfg.merge(CaloFwdTopoTowerCfg(cfgFlags,CaloTopoClusterContainerKey="CaloCalTopoClusters"))

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    toAOD = [ "CaloClusterContainer#CaloCalFwdTopoTowers", "CaloClusterContainer#CaloCalFwdTopoTowersAux."]
    toAOD += ["xAOD::TruthEventContainer#TruthEvents", "xAOD::TruthEventAuxContainer#TruthEventsAux."]
    toAOD += [ "xAOD::VertexContainer#PrimaryVertices","xAOD::VertexAuxContainer#PrimaryVerticesAux." ]
    toAOD += ["xAOD::MuonSegmentContainer#MuonSegments", "xAOD::MuonSegmentAuxContainer#MuonSegmentsAux."]
    toAOD += [ "xAOD::TruthParticleContainer#TruthParticles","xAOD::TruthParticleAuxContainer#TruthParticlesAux." ] 

    cfg.merge(addToAOD(cfgFlags, toAOD))

    return cfg

def configureMetaDataForPFlowCfg(cfgFlags):

    cfg = ComponentAccumulator()

    from AthenaConfiguration.Enums import MetadataCategory
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    cfg.merge(
        SetupMetaDataForStreamCfg(
            cfgFlags,
            "AOD",
            createMetadata=[
                MetadataCategory.ByteStreamMetaData,
                MetadataCategory.LumiBlockMetaData,
                MetadataCategory.TruthMetaData,
                MetadataCategory.IOVMetaData,
            ],
        )
    )

    return cfg

def configureRecoForPFlowCfg(cfgFlags):

    #Given we rebuild topoclusters from the ESD, we should also redo the matching between topoclusters and muon clusters.
    #The resulting links are used to create the global GPF muon-FE links.
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg = ComponentAccumulator()    
    cfg.addEventAlgo(CompFactory.ClusterMatching.CaloClusterMatchLinkAlg("MuonTCLinks", ClustersToDecorate="MuonClusterCollection"))

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4LCTopo
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4EMPFlow) )     
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4LCTopo) )
    
    #Now do MET config

    #The MET soft term needs the EM and LC origin topoclusters.
    #Since we rebuilt the topoclusters, we must also rebuild these.
    from JetRecConfig.JetRecConfig import JetInputCfg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    cfg.merge(JetInputCfg(cfgFlags,cst.EMTopoOrigin))
    cfg.merge(JetInputCfg(cfgFlags,cst.LCTopoOrigin))

    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    cfg.merge(PFGlobalFlowElementLinkingCfg(cfgFlags))

    from tauRec.TauConfig import TauReconstructionCfg
    cfg.merge(TauReconstructionCfg(cfgFlags))

    from eflowRec.PFTauRemaps import PFTauRemaps
    tauRemaps = PFTauRemaps()
    for mapping in tauRemaps:
        cfg.merge(mapping)

    #Now build the pflow MET association map and then add the METMaker algorithm
    from METReconstruction.METAssociatorCfg import METAssociatorCfg
    cfg.merge(METAssociatorCfg(cfgFlags, 'AntiKt4EMPFlow'))

    from METUtilities.METMakerConfig import getMETMakerAlg
    metCA=ComponentAccumulator()    
    metCA.addEventAlgo(getMETMakerAlg('AntiKt4EMPFlow'))
    cfg.merge(metCA)

    #Add metadata data to xAOD such that we can run workflows such as creating physval.root 
    # from the output AOD or making ntuples from the output AOD
    cfg.merge( configureMetaDataForPFlowCfg(cfgFlags) )

    return cfg

