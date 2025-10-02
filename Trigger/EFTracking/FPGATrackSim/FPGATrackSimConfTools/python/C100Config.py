# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
def WriteToAOD(flags, finalTrackParticles = ''): #  store xAOD containers in AOD file
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory
    
    result.merge( SetupMetaDataForStreamCfg( flags,"AOD", 
                                            createMetadata=[
                                                MetadataCategory.ByteStreamMetaData,
                                                MetadataCategory.LumiBlockMetaData,
                                                MetadataCategory.TruthMetaData,
                                                MetadataCategory.IOVMetaData,],)
                )
    
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    toAOD = [f"xAOD::TrackParticleContainer#{finalTrackParticles}",f"xAOD::TrackParticleAuxContainer#{finalTrackParticles}Aux."]
    if flags.Trigger.FPGATrackSim.writeClustersToAOD:
        toAOD += ["xAOD::PixelClusterContainer#ITkPixelClusters","xAOD::PixelClusterAuxContainer#ITkPixelClustersAux.",
                "xAOD::StripClusterContainer#ITkStripClusters","xAOD::StripClusterAuxContainer#ITkStripClustersAux."]    
    result.merge(addToAOD(flags, toAOD))

    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    TrackParticlePrefix="ActsFast"
    
    flags = initConfigFlags()
    from ActsConfig.ActsCIFlags import actsLegacyWorkflowFlags
    actsLegacyWorkflowFlags(flags)
    
    # IDTPM flags
    from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags
    flags = initializeIDTPMConfigFlags(flags)
    
    flags.fillFromArgs()
    flags = initializeIDTPMTrkAnaConfigFlags(flags) # should run after `fillFromArgs`
    
    ## override respective configurations from trkAnaCfgFile
    flags.PhysVal.IDTPM.TrkAnaEF.TrigTrkKey = f"{TrackParticlePrefix}TrackParticles"
    if not flags.Trigger.FPGATrackSim.writeClustersToAOD:
        flags.PhysVal.IDTPM.TrkAnaDoubleRatio.TrigTrkKey = f"{TrackParticlePrefix}TrackParticles"

    flags.Debug.DumpEvtStore=True
    flags.Concurrency.NumThreads=0
    flags.Concurrency.NumConcurrentEvents=0
    flags.Concurrency.NumProcs=0
    
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)
    flags.dump()
    
    acc=MainServicesCfg(flags)
    
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        acc.merge(GEN_AOD2xAODCfg(flags))

    from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
    acc.merge(addTruthPileupJetsToOutputCfg(flags))
    
    if flags.Detector.EnableCalo:
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        acc.merge(CaloRecoCfg(flags))

    if not flags.Reco.EnableTrackOverlay:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        acc.merge(InDetTrackRecoCfg(flags))
        from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )
        
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimDataPrepConnectToFastTracking
    acc.merge(FPGATrackSimDataPrepConnectToFastTracking(flags, FinalTracks=f"{TrackParticlePrefix}"))
        
    if flags.Trigger.FPGATrackSim.writeClustersToAOD:
        acc.merge(WriteToAOD(flags, finalTrackParticles=f"{TrackParticlePrefix}TrackParticles"))
    else:
        # IDTPM running
        from InDetTrackPerfMon.InDetTrackPerfMonConfig import InDetTrackPerfMonCfg
        acc.merge( InDetTrackPerfMonCfg(flags) )
      
    acc.store(open('AnalysisConfig.pkl','wb'))
    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess() is True, "Application execution did not succeed"