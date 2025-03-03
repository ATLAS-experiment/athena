# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()

    cfgFlags.Concurrency.NumThreads=1
    cfgFlags.Exec.MaxEvents=100
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD.31373517._000035.pool.root.1"]
    cfgFlags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.Calo.TopoCluster.addCPData=True
    cfgFlags.Calo.TopoCluster.doTopoClusterLocalCalib=True
    cfgFlags.Calo.TopoCluster.addCalibrationHitDecoration=True
    cfgFlags.PF.addCPData=True
    cfgFlags.PF.useTruthForChargedShowerSubtraction=True
    cfgFlags.PF.useTruthCheating=True
    cfgFlags.PF.useTrackClusterTruthMatching=True
    cfgFlags.Tau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
    cfgFlags.fillFromArgs()
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))
    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags,runTauReco=True))
    
    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps(cfg, 'AOD')
    for mapping in list_remaps:
        cfg.merge(mapping)    

    from CaloCalibHitRec.CaloCalibHitDecoratorCfg import CaloCalibHitDecoratorCfg 
    #switch this to use uncalibrated topoclusters which eflowRec will use.
    cfg.merge(CaloCalibHitDecoratorCfg(cfgFlags,name="CaloCalibHitDecoratorEMAlgorithm",CaloClusterWriteDecorHandleKey_NLeadingTruthParticles="CaloTopoClusters.calclus_NLeadingTruthParticleBarcodeEnergyPairs"))

    from PFlowUtils.configureRecoForPFlow import configureRecoForPFlowCfg
    cfg.merge(configureRecoForPFlowCfg(cfgFlags))

    from PFlowUtils.PFlowCalibHitDecoratorCfg import PFlowCalibHitDecoratorCfg
    cfg.merge(PFlowCalibHitDecoratorCfg(cfgFlags)) #

    cfg.getEventAlgo("PFlowCalibPFODecoratorAlgorithm").PFOWriteDecorHandleKey_NLeadingTruthParticles="GlobalNeutralParticleFlowObjects.calpfo_NLeadingTruthParticleBarcodeEnergyPairs"

    from JetRecConfig.StandardSmallRJets import AntiKt4Truth
    from JetRecConfig.JetRecConfig import JetRecCfg
    cfg.merge(JetRecCfg(cfgFlags,AntiKt4Truth)) 

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    toESDAndAOD = [ "xAOD::JetContainer#AntiKt4EMPFlowJets","xAOD::JetAuxContainer#AntiKt4EMPFlowJetsAux.-PseudoJet" ]
    toESDAndAOD +=  [ "xAOD::JetContainer#AntiKt4EMTopoJets","xAOD::JetAuxContainer#AntiKt4EMTopoJetsAux.-PseudoJet" ]
    toESDAndAOD +=  [ "xAOD::JetContainer#AntiKt4TruthJets","xAOD::JetAuxContainer#AntiKt4TruthJetsAux.-PseudoJet" ]
    toESDAndAOD += [ "xAOD::TruthParticleContainer#TruthParticles","xAOD::TruthParticleAuxContainer#TruthParticlesAux." ] 
    toESDAndAOD += [ "xAOD::FlowElementContainer#GlobalChargedParticleFlowObjects","xAOD::FlowElementAuxContainer#GlobalChargedParticleFlowObjectsAux." ] 
    toESDAndAOD += [ "xAOD::FlowElementContainer#GlobalNeutralParticleFlowObjects","xAOD::FlowElementAuxContainer#GlobalNeutralParticleFlowObjectsAux." ]
    toESDAndAOD += [ "xAOD::FlowElementContainer#CHSGChargedParticleFlowObjects","xAOD::ShallowAuxContainer#CHSGChargedParticleFlowObjectsAux." ]
    toESDAndAOD += [ "xAOD::FlowElementContainer#CHSGNeutralParticleFlowObjects","xAOD::ShallowAuxContainer#CHSGNeutralParticleFlowObjectsAux." ]
    toESDAndAOD += [ "xAOD::VertexContainer#PrimaryVertices","xAOD::VertexAuxContainer#PrimaryVerticesAux." ]
    toESDAndAOD += [ "xAOD::EventShape#Kt4EMPFlowEventShape","xAOD::EventShapeAuxInfo#Kt4EMPFlowEventShapeAux." ]
    toESDAndAOD += [ "xAOD::EventShape#Kt4EMTopoOriginEventShape","xAOD::EventShapeAuxInfo#Kt4EMTopoOriginEventShapeAux." ]

    cfg.merge(addToAOD(cfgFlags,toESDAndAOD))

    #Given we rebuild topoclusters from the ESD, we should also redo the matching between topoclusters and muon clusters.
    #The resulting links are used to create the global GPF muon-FE links.
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()    
    result.addEventAlgo(CompFactory.ClusterMatching.CaloClusterMatchLinkAlg("MuonTCLinks", ClustersToDecorate="MuonClusterCollection"))
    cfg.merge(result)

    cfg.run()
